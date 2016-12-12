/* include area */
#include <argp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codecs/ascii.h"
#include "codecs/dummy.h"
#include "match.h"
#include "window.h"


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


typedef struct
{
  const window_t *w;
  char c;

} ml_update_cb_ctx_t;


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


static bool _find_match( size_t from, size_t *pos, const window_t *window, char c )
{
  for( size_t i = from; i < window_get_size( window ); i++ )
  {
    char wchar;
    if( !window_read( window, &wchar, i ) )
      ABORT( "Unexpected window size" );

    if( wchar == c )
    {
      *pos = i;
      return true;
    }
  }

  return false;
}


static size_t _find_matches( const window_t *w, match_list_t *ml, char c )
{
  match_t m;
  size_t wpos = 0, found_matches = 0;
  while( _find_match( wpos, &wpos, w, c ) )
  {
    m.pos = wpos;
    m.len = 1;

    /* adds a match into the list */
    if( !match_list_append( ml, &m ) )
      ABORT( "Too many elements" );

    found_matches++;
    wpos++;
  }

  return found_matches;
}


static bool _ml_update_cb( match_t *m, void *cb_ctx )
{
  ml_update_cb_ctx_t *ctx = cb_ctx;

  char wchar;
  if( !window_read( ctx->w, &wchar, m->pos ) )
    ABORT( "Bad read" );

  /* if it doesn't match anymore, removes the match from the list */
  if( wchar != ctx->c )
    return false;

  /* still matching at that position, increments the match length */
  m->len += 1;

  return true;
}


static size_t _update_matches( match_list_t *ml, const window_t *w, char c, match_t *best_match )
{
  ml_update_cb_ctx_t cb_ctx = {
    .c = c,
    .w = w
  };

  /* gets the first match in the list, in case there are no more matches left in the window */
  if( !match_list_get( ml, 0, best_match ) )
    ABORT( "Failed getting the first match" );

  /* updates the list removing all matches that are not valid anymore */
  return match_list_update( ml, _ml_update_cb, &cb_ctx );
}


static void _print_window( const window_t *w )
{
  char c;

  if( w->buffer_size - window_get_size( w ) )
    printf( "[%*c", (int)( w->buffer_size - window_get_size( w ) ), ' ' );
  else
    printf( "[" );

  for( size_t i = 0; i < window_get_size( w ); i++ )
  {
    if( !window_read( w, &c, i ) )
      ABORT( "Expected char" );

    printf( "%c", c );
  }

  printf( "]");
}


static void _fsm( FILE *input, window_t *w, window_t *read, match_list_t *ml, codec_t *codec )
{
  #define INIT 0
  #define READ 1
  #define FIND 2
  #define UPDT 3
  #define EXIT 4
  #define PRNT 5

  #define MIN_MLEN 2

  bool stop = false;
  int current;
  match_t best;

  int state = INIT;

  printf( "windw|buf| in | out\n" );
  printf( "===================" );
  while( !stop )
  {
    switch( state )
    {
      case READ:
        window_append( w, current );

      case INIT:
        printf( "\n" );
        _print_window( w );
        _print_window( read );
        current = getc( input );

        if( current == EOF || feof( input ) )
        {
          state = EXIT;
          printf( " EOF -> " );
        }
        else
        {
          state = ( ml->num_elems > 0 ) ? UPDT : PRNT;
          printf( " %c -> ", current );
        }
        break;

      case PRNT:
      print:
        if( window_get_size( read ) > 0 )
        {
          if( window_get_size( read ) < MIN_MLEN )
          {
            char c;
            for( size_t i = 0; i < window_get_size( read ); i++ )
            {
              ASSERT( window_read( read, &c, i ), "Expected a char" );
              codec->write_literal( codec, c );
            }
          }
          else
            codec->write_match( codec, best );
          window_clear( read );
        }
        state = FIND;
        break;

      case FIND:
        if( _find_matches( w,  ml, current ) > 0 )
        {
          window_append( read, current );
          char pos[window_get_size( w ) + 1];
          memset( pos, ' ', sizeof( pos ) );
          pos[window_get_size( w )] = '\0';
          for( size_t i = 0; i < ml->num_elems; i++ )
          {
            match_t m;
            ASSERT( match_list_get( ml, i, &m ), "bla" );
            pos[m.pos] = '^';
          }
          printf( "\n %s", pos );
        }
        else
          codec->write_literal( codec, current );

        state = READ;
        break;

      case UPDT:
        state = READ;
        if( _update_matches( ml,  w, current, &best ) > 0 )
          window_append( read, current );
        else
          state = PRNT;
        break;

      case EXIT:
        stop = true;
        goto print;
        break;
    }
  }
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
  /* window containing a limited sequence of characters seen so far */
  window_t window;
  if( !window_create( &window, wsize ) )
    ABORT( "Malloc error" );

  /* contains the characters matched that could not end up in a match because of the match's length
   **/
  window_t read;
  if( !window_create( &read, min_match ) )
    ABORT( "Malloc error" );

  match_list_t match_list;
  if( !match_list_init( &match_list, wsize ) )
    ABORT( "Malloc error" );

  /* sets the appropriate codec */
  codec_t codec = ascii ? ascii_codec_create() : dummy_codec_create();

  /* runs the LZ algorithm */
  _fsm( input, &window, &read, &match_list, &codec );

  /* releases all resources */
  codec.close( &codec );
  match_list_uninit( &match_list );
  window_destroy( &read );
  window_destroy( &window );
}


int main( int argc, char **argv )
{
  /* initializes the arguments with the default values */
  args_t arguments = {
    .verbose = false,
    .ascii = false,
    .input_file = "stdin",
    .output_file = "stdout",
    .window = 4,
    .min_match = 2
  };

  /* parses the user arguments */
  argp_parse( &argp, argc, argv, 0, 0, &arguments );

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
