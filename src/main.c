/* include area */
#include <argp.h>
#include "codecs/ascii.h"
#include "codecs/binary.h"
#include "lzss.h"


/** Wraps a string with the red and bold ANSI color codes. */
#define RED( text )  "\033[1;31m" text "\033[0m"


/** Prints a message and aborts the program. */
#define ABORT( msg )\
  do {\
    printf( RED( "[ABORTED]" ) " %s (%s:%d)\n", msg, __FILE__, __LINE__ );\
    exit(1);\
  } while(0)


/** Aborts the program if \c expression evaluates to \c false. */
#define ASSERT( expression, msg )\
  do {\
    if( !( expression ) ) ABORT( msg );\
  } while(0)


/* Used by main to communicate with parse_opt. */
typedef struct
{
  /* flags */
  bool verbose, ascii;

  /* file where the output is stored */
  char *output_file;

  /* file to read data from */
  char *input_file;

  /* window size */
  size_t window;

  /* minimum match length */
  size_t min_match;

} args_t;


/** Program version. */
const char *argp_program_version = "LZSS 1.0.0";

/** Program documentation. */
static char doc[] = "LZSS compressor";

/* Accepted options. */
static struct argp_option options[] = {
  { "verbose",  'v', 0,      0,  "Produce verbose output" },
  { "ascii",    'a', 0,      0,  "Output in ASCII format instead of binary" },
  { "input",    'i', "FILE", 0,  "Compress from FILE instead of stdin" },
  { "output",   'o', "FILE", 0,  "Output to FILE instead of standard output" },
  { 0 }
};


/* Parse a single option. */
static error_t parse_opt( int key, char *arg, struct argp_state *state )
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  args_t *arguments = state->input;

  switch( key )
  {
    case 'v':
      arguments->verbose = true;
      break;

    case 'a':
      arguments->ascii = true;
      break;

    case 'o':
      arguments->output_file = arg;
      break;

    case 'i':
      arguments->input_file = arg;
      break;

    case ARGP_KEY_ARG:
      if( state->arg_num >= 0 )
        /* Too many arguments. */
        argp_usage( state );
      break;

    case ARGP_KEY_END:
      if( state->arg_num != 0 )
        /* Not enough arguments. */
        argp_usage( state );
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}


/* argp parser. */
static struct argp argp = { options, parse_opt, NULL, doc };


/**
 * Callback used by the codecs to output data.
 * @param  buffer      Pointer to the encoded data.
 * @param  buffer_size Size of the data to encode.
 * @param  ctx         FILE stream.
 * @return             \c true on success, \c false otherwise.
 */
static bool _codec_out_cb( const void *buffer, size_t buffer_size, void *ctx )
{
  /* the callabck context is the file stream */
  return( fwrite( buffer, 1, buffer_size, ctx ) == buffer_size );
}


/** Compress the file \a input and save it in \a output.
 *
 *  \param output File where the output is written.
 *  \param input File to compress.
 *  \param wsize Window size.
 *  \param min_match Minimum match length.
 *  \param min_match Maximum match length.
 */
void compress( FILE *output, FILE *input, size_t wsize, size_t min_match, size_t max_match, bool ascii )
{
  /* sets the appropriate codec */
  codec_t *codec = ascii ? ascii_codec_create( _codec_out_cb,
                                               output,
                                               min_match,
                                               max_match,
                                               wsize ) :
                           binary_codec_create( _codec_out_cb,
                                                output,
                                                min_match,
                                                max_match,
                                                wsize );
  if( !codec )
    ABORT( "Codec init error" );

  lzss_t lz;
  lzss_error_t error = lzss_init( &lz, wsize, min_match, max_match, codec );
  if( error != lzss_error_no_error )
    ABORT( "Init error." );

  byte input_buffer[4096];
  size_t bytes_read = 0;

  /* runs the LZ algorithm */
  do
  {
    bytes_read = fread( input_buffer, sizeof( byte ), sizeof( input_buffer ), input );
    error = lzss_compress( &lz, input_buffer, bytes_read );
    if( error != lzss_error_no_error )
      ABORT( "Compress error." );
  }
  while( bytes_read > 0 );

  /* releases all resources */
  error = lzss_end( &lz );
  if( error != lzss_error_no_error )
    ABORT( "Could not finish the compression correctly." );

  lzss_uninit( &lz );
  codec->destroy( codec );
}


#ifdef __TESTS__
int _fake_main( int argc, char **argv )
#else
int main( int argc, char **argv )
#endif
{
  /* initializes the arguments with the default values */
  args_t arguments = {
    .verbose = false,
    .ascii = false,
    .input_file = "stdin",
    .output_file = "stdout",
    .window = 10 << 20,
    .min_match = 8
  };

  /* parses the user arguments */
  argp_parse( &argp, argc, argv, 0, 0, &arguments );

  if( arguments.verbose )
    printf( "INPUT FILE = %s\nOUTPUT_FILE = %s\nVERBOSE = %s\nASCII = %s\n",
            arguments.input_file,
            arguments.output_file,
            arguments.verbose ? "yes" : "no",
            arguments.ascii ? "yes" : "no" );

  FILE *input = stdin;
  if( strcmp( arguments.input_file, "stdin" ) != 0 )
  {
    input = fopen( arguments.input_file, "rb" );
    if( input == NULL )
    {
      printf( "invalid input file\n" );
      exit(1);
    }
  }

  FILE *output = stdout;
  if( strcmp( arguments.output_file, "stdout" ) != 0 )
  {
    output = fopen( arguments.output_file, "wb" );
    if( output == NULL )
    {
      printf( "invalid output file\n" );
      fclose( input );
      exit(1);
    }
  }

  compress( output, input, arguments.window, arguments.min_match, 100, arguments.ascii );

  fclose( input );
  fclose( output );

  return 0;
}
