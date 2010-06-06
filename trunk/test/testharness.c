/* ****************************************************************************
 * Format - lightweight string formatting library.
 * Copyright (C) 2010, Neil Johnson
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms,
 * with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ************************************************************************* */


/*****************************************************************************/
/* System Includes                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "format.h"

/*****************************************************************************/
/* Project Includes                                                          */
/*****************************************************************************/

/**
    Set the size of the test buffers
**/
#define BUF_SZ      ( 256 )

static char buf[BUF_SZ];
static int f = 0;

/**
    Wrapper macro to standardise checking of test results.
    
    @param exs              Expected result string
    @param rtn              Expected return value
    @param fmt              Test format string
    @param args...          Argument list
**/
#define TEST(exs,rtn,fmt,args...)   do {                                    \
            int r = test_sprintf(buf,(fmt),## args);                        \
            printf( "[Test  @ %3d] ", __LINE__ );                           \
            if ( r != (rtn) )                                               \
                {printf("**** FAIL: returned %d, expected %d.", r, (rtn) );f|=1;} \
            else if ( strcmp( (exs), buf ) )                                \
                {printf("**** FAIL: produced \"%s\", expected \"%s\".", buf,(exs));f|=1;}\
            else                                                            \
                printf("PASS");                                             \
            printf("\n");                                                   \
            } while( 0 );
            
/**
    Check if two integers are the same and print out accordingly.
**/
#define CHECK(a,b)      do { printf("[Check @ %3d] ", __LINE__ );           \
                            if ((a)==(b))                                   \
                                printf( "PASS");                            \
                            else {printf("**** FAIL: got %d, expected %d",(a),(b));f|=1;}\
                            printf("\n");                                   \
                        }while(0);

/*****************************************************************************/
/* Private functions.  Declare as static.                                    */
/*****************************************************************************/

/*****************************************************************************/
/**
    Format consumer function to write characters to a user-supplied buffer.
    
    @param op       Opaque pointer (not used)
    @param buf      Pointer to buffer of characters to consume from
    @param n        Number of characters from @p buf to consume
    
    @returns NULL if failed, else non-NULL.
**/
static void * bufwrite( void * memptr, const char * buf, size_t n )
{
    return ( memcpy( memptr, buf, n ) + n ); 
}

/*****************************************************************************/
/**
    Example use of format() to implement the standard sprintf()
    
    @param buf      Pointer to receiving buffer
    @param fmt      Format string
    
    @returns Number of characters printed, or -1 if failed.
**/
int test_sprintf( char *buf, const char *fmt, ... )
{
    va_list arg;
    int done;
    
    va_start ( arg, fmt );
    done = format( bufwrite, buf, fmt, arg );
    if ( 0 <= done )
        buf[done] = '\0';
    va_end ( arg );
    
    return done;
}

/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/**
    Execute tests on plain strings
**/
static void test( void )
{
    printf( "Testing basic strings\n" );
    
    TEST( "", EXBADFORMAT, NULL );
    
    /* Empty string */
    TEST( "", 0, "" );
    
    /* Basic tests */
    TEST( "a", 1, "a" );
    TEST( "abc", 3, "abc" );
    
    /* Long string */
    TEST( "abcdefghijabcdefghijabcdefghijabcdefghijabcdefghij"
          "abcdefghijabcdefghijabcdefghijabcdefghijabcdefghij",
          100,
          "abcdefghijabcdefghijabcdefghijabcdefghijabcdefghij"
          "abcdefghijabcdefghijabcdefghijabcdefghijabcdefghij" );
          
    /* Escape characters */
    TEST( "\a\b\f\n\r\t\v", 7, "\a\b\f\n\r\t\v" );
    TEST( "\'\"\\\?", 4, "\'\"\\\?" );
    TEST( "\123\x69", 2, "\123\x69" );
}

/*****************************************************************************/
/**
    Execute tests on '%' conversion specifier
**/
static void test_pc( void )
{
    printf( "Testing \"%%%%\"\n" );
    
    /* Basic test */
    TEST( "%", 1, "%%" );
    
    /* Check all flags, precision, width, length are ignored */
    TEST( "%", 1, "%-+ #0!12.24h%" );
    
    /* Check sequential conversions */
    TEST( "%c", 2, "%%c" );
    TEST( "%%%", 3, "%%%%%%" );
    TEST( "% % %", 5, "%% %% %%" );
}

/*****************************************************************************/
/**
    Execute tests on 'c' conversion specifier
**/
static void test_c( void )
{
    printf( "Testing \"%%c\"\n" );
    
    /* Basic test */
    TEST( "a", 1, "%c", 'a' );
    
    /* Check all flags, precision, width, length are ignored */
    TEST( "a", 1, "%-+ #0!12.24hc", 'a' );
    TEST( "a", 1, "%-+ #0!12.24lc", 'a' );
    
    /* Check sequential conversions */
    TEST( "ac", 2, "%cc", 'a' );
    TEST( "abc", 3, "%c%c%c", 'a', 'b', 'c' );
    TEST( "a b c", 5, "%c %c %c", 'a', 'b', 'c' );
}

/*****************************************************************************/
/**
    Execute tests on 'n' conversion specifier
**/
static void test_n( void )
{
    int n;
    short s;
    long l;
    
    printf( "Testing \"%%n\"\n" );
    
    /* Basic positional tests */
    TEST( "hello", 5, "hello%n", &n ); CHECK( n, 5 );
    TEST( "hello", 5, "hel%nlo", &n ); CHECK( n, 3 );
    TEST( "hello", 5, "%nhello", &n ); CHECK( n, 0 );
    
    /* Length modifiers */
    TEST( "hello", 5, "hello%ln", &l ); CHECK( l, 5 );
    TEST( "hello", 5, "hello%hn", &s ); CHECK( s, 5 );
    
    /* NULL pointer - should silently ignore */
    TEST( "hello", 5, "hello%n", NULL );
    TEST( "hello", 5, "hello%hn", NULL );
    TEST( "hello", 5, "hello%ln", NULL );
    
    /* Check all flags, precision, and width are ignored */
    TEST( "hello", 5, "hello%-+ #0!12.24n", &n ); CHECK( n, 5 );
}

/*****************************************************************************/
/**
    Execute tests on 's' conversion specifier
**/
static void test_s( void )
{
    printf( "Testing \"%%s\"\n" );
    
    /* Basic string operations */
    TEST( "hello", 5, "%s", "hello" );
    TEST( "goodbye", 7, "%sbye", "good" );
    
    TEST( "   hello", 8, "%8s", "hello" );
    TEST( "hello   ", 8, "%-8s", "hello" );
    TEST( "     hel", 8, "%8.3s", "hello" );
    TEST( "hel     ", 8, "%-8.3s", "hello" );
    TEST( "hel", 3, "%.3s", "hello" );
        
    /* NULL pointer handled specially */
    TEST( "(null)", 6, "%s", NULL );

    /* Check unused flags and lengths are ignored */
    TEST( "hello", 5, "%+ #0!ls", "hello" );
    TEST( "hello", 5, "%+ #0!hs", "hello" );
}

/*****************************************************************************/
/**
    Execute tests on 'p' conversion specifier
    
    *May* work for 16,32 and 64-bit pointers.  Its a bit iffy really.
    On really weird architectures this is just wild.
**/
static void test_p( void )
{
    int ptr_size = sizeof( int * );
    int * p0 = (int *)0x0;
    int * p1 = (int *)0x1234;                          
    int * p2 = (int *)(-1);
    
    printf( "Testing \"%%p\" on platform with %d-byte pointers\n", ptr_size );
    
    if ( ptr_size == 2 )
    {
        TEST( "0x0000", 6, "%p", p0 );
        TEST( "0x1234", 6, "%p", p1 );
        TEST( "0xFFFF", 6, "%p", p2 );
        
        /* Check all flags, precision, width, length are ignored */
        TEST( "0xFFFF", 6, "%-+ #0!12.24lp", p2 );
        TEST( "0xFFFF", 6, "%-+ #0!12.24hp", p2 );
    }
    else if ( ptr_size == 4 )
    {       
        TEST( "0x00000000", 10, "%p", p0 );
        TEST( "0x00001234", 10, "%p", p1 );
        TEST( "0xFFFFFFFF", 10, "%p", p2 );
        
        /* Check all flags, precision, width, length are ignored */
        TEST( "0xFFFFFFFF", 10, "%-+ #0!12.24lp", p2 );
        TEST( "0xFFFFFFFF", 10, "%-+ #0!12.24hp", p2 );
    }
    else if ( ptr_size == 8 )
    {
        TEST( "0x0000000000000000", 18, "%p", p0 );
        TEST( "0x0000000000001234", 18, "%p", p1 );
        TEST( "0xFFFFFFFFFFFFFFFF", 18, "%p", p2 );
        
        /* Check all flags, precision, width, length are ignored */
        TEST( "0xFFFFFFFFFFFFFFFF", 18, "%-+ #0!24.48lp", p2 );
        TEST( "0xFFFFFFFFFFFFFFFF", 18, "%-+ #0!24.48hp", p2 );
    }
    else
    {
        printf( "ERROR: unknown pointer size (%d bytes)\n", ptr_size );
        f |= 1;
    }
}

/*****************************************************************************/
/**
    Execute tests on 'd' and 'i' conversion specifiers.

d,i        The int argument is converted to signed decimal in the style [-]dddd. 
           The precision specifies the minimum number of digits to appear; if
           the value being converted can be represented in fewer digits, it is 
           expanded with leading zeros. The default precision is 1. The result 
           of converting a zero value with a precision of zero is no characters.
**/
static void test_di( void )
{
    short int si = 24;
    long int  li = 1234567890L;
    
    printf( "Testing \"%%d\" and \"%%i\"\n" );
    
    TEST( "0", 1, "%d", 0 );
    TEST( "1234", 4, "%d", 1234 );
    TEST( "-1234", 5, "%d", -1234 );

    /* 0 value with 0 precision produces no characters */
    TEST( "", 0, "%.0d", 0 );

    /* Precision sets minimum number of digits, zero-padding if necessary */
    TEST( "001234", 6, "%.6d", 1234 );
    
    /* Width sets minimum field width */
    TEST( "  1234", 6, "%6d", 1234 );
    TEST( " -1234", 6, "%6d", -1234 );
    TEST( "1234", 4, "%2d", 1234);
    
    /* Precision sets minimum number of digits for the value */
    TEST( "001234", 6, "%.6d", 1234 );
    
    /* '-' flag */
    TEST( "1234  ", 6, "%-6d", 1234 );
    TEST( "-1234 ", 6, "%-6d", -1234 );
        
    /* '0' flag */
    TEST( "001234", 6, "%06d", 1234 );
    TEST( "1234  ", 6, "%-06d", 1234 ); /* '-' kills '0' */
    TEST( "  1234", 6, "%06.1d", 1234 ); /* prec kills '0' */

    /* '+' */
    TEST( "+1234", 5, "%+d", 1234 );
    TEST( "-1234", 5, "%+d", -1234);
    
    /* space */
    TEST( " 1234", 5, "% d", 1234 );
    TEST( "-1234", 5, "% d", -1234 );
    TEST( " ", 1, "% .0d", 0 );
    
    TEST( "+1234", 5, "%+ d", 1234 ); /* '+' kills space */
    TEST( "-1234", 5, "%+ d", -1234); /* '+' kills space */
    TEST( "+", 1, "%+ .0d", 0 ); /* '+' kills space */
    
    /* no effect */
    TEST( "1234", 4, "%!#d", 1234 );
    
    /* lengths */
    TEST( "24", 2, "%hd", si );
    TEST( "1234567890", 10, "%ld", li );
}


/*****************************************************************************/
/**
    Execute tests on b,o,u,x,X conversion specifiers.

b,o,u,x,X  The unsigned int argument is converted to unsigned binary (b), 
           unsigned octal (o), unsigned decimal (u), or unsigned hexadecimal 
           notation (x or X) in the style dddd; the letters "abcdef" are used 
           for x conversion and the letters "ABCDEF" for X conversion. The 
           precision specifies the minimum number of digits to appear; if the 
           value being converted can be represented in fewer digits, it is 
           expanded with leading zeros.  The default precision is 1. The result
           of converting a zero value with a precision of zero is no characters.
**/
static void test_bouxX( void )
{
    printf( "Testing \"%%b\", \"%%o\", \"%%u\", \"%%x\" and \"%%X\"\n" );
    
    TEST( "0", 1, "%b", 0 );
    TEST( "0", 1, "%o", 0 );
    TEST( "0", 1, "%u", 0 );
    TEST( "0", 1, "%x", 0 );
    TEST( "0", 1, "%X", 0 );
    
    TEST( "1101", 4, "%b", 13 );
    TEST( "1234", 4, "%o", 01234 );
    TEST( "1234", 4, "%u", 1234 );
    TEST( "1234abcd", 8, "%x", 0x1234abcd );
    TEST( "1234ABCD", 8, "%X", 0x1234ABCD );
    
    /* 0 value with 0 precision produces no characters */
    TEST( "", 0, "%.0b", 0 );
    TEST( "", 0, "%.0o", 0 );
    TEST( "", 0, "%.0u", 0 );
    TEST( "", 0, "%.0x", 0 );
    TEST( "", 0, "%.0X", 0 );
    
    /* Precision sets minimum number of digits, zero-padding if necessary */
    TEST( "001101", 6, "%.6b", 13 );
    TEST( "001234", 6, "%.6o", 01234 );
    TEST( "001234", 6, "%.6u", 1234 );
    TEST( "001234abcd", 10, "%.10x", 0x1234abcd );
    TEST( "001234ABCD", 10, "%.10X", 0x1234ABCD );
    
    /* Width sets minimum field width */
    TEST( "  1101", 6, "%6b", 13 );
    TEST( "1101", 4, "%2b", 13);
    TEST( "  1234", 6, "%6o", 01234 );
    TEST( "1234", 4, "%2o", 01234);
    TEST( "  1234", 6, "%6u", 1234 );
    TEST( "1234", 4, "%2u", 1234);
    TEST( "  1234abcd", 10, "%10x", 0x1234abcd );
    TEST( "1234abcd", 8, "%2x", 0x1234abcd);
    TEST( "  1234ABCD", 10, "%10X", 0x1234ABCD );
    TEST( "1234ABCD", 8, "%2X", 0x1234ABCD);
    
    /* Precision sets minimum number of digits for the value */
    TEST( "001101", 6, "%.6b", 13 );
    TEST( "001234", 6, "%.6o", 01234 );
    TEST( "001234", 6, "%.6u", 1234 );
    TEST( "001234abcd", 10, "%.10x", 0x1234abcd );
    TEST( "001234ABCD", 10, "%.10X", 0x1234abcd );
    
    /* '-' flag */
    TEST( "1101  ", 6, "%-6b", 13 );
    TEST( "1234  ", 6, "%-6o", 01234 );
    TEST( "1234  ", 6, "%-6u", 1234 );
    TEST( "1234abcd  ", 10, "%-10x", 0x1234abcd );
    TEST( "1234ABCD  ", 10, "%-10X", 0x1234abcd );
    
    /* '0' flag */
    TEST( "001101", 6, "%06b", 13 );
    TEST( "1101  ", 6, "%-06b", 13 ); /* '-' kills '0' */
    TEST( "  1101", 6, "%06.1b", 13 ); /* prec kills '0' */
    TEST( "001234", 6, "%06o", 01234 );
    TEST( "1234  ", 6, "%-06o", 01234 ); /* '-' kills '0' */
    TEST( "  1234", 6, "%06.1o", 01234 ); /* prec kills '0' */
    TEST( "001234", 6, "%06u", 1234 );
    TEST( "1234  ", 6, "%-06u", 1234 ); /* '-' kills '0' */
    TEST( "  1234", 6, "%06.1u", 1234 ); /* prec kills '0' */
    TEST( "001234abcd", 10, "%010x", 0x1234abcd );
    TEST( "1234abcd  ", 10, "%-010x", 0x1234abcd ); /* '-' kills '0' */
    TEST( "  1234abcd", 10, "%010.1x", 0x1234abcd ); /* prec kills '0' */
    TEST( "001234ABCD", 10, "%010X", 0x1234abcd );
    TEST( "1234ABCD  ", 10, "%-010X", 0x1234abcd ); /* '-' kills '0' */
    TEST( "  1234ABCD", 10, "%010.1X", 0x1234abcd ); /* prec kills '0' */
    
    /* Alternate form */
    TEST( "0", 1, "%#b", 0 );
    TEST( "0", 1, "%#o", 0 );
    TEST( "0", 1, "%#x", 0 );
    TEST( "0", 1, "%#X", 0 );
    
    TEST( "0b1101", 6, "%#b", 13 );
    TEST( "01234", 5, "%#o", 01234 );
    TEST( "0x1234abcd", 10, "%#x", 0x1234abcd );
    TEST( "0X1234ABCD", 10, "%#X", 0x1234abcd );
    
    /* Alternate with ! */
    TEST( "0b0", 3, "%!#b", 0 );
    TEST( "0", 1, "%!#o", 0 );
    TEST( "0x0", 3, "%!#x", 0 );
    TEST( "0x0", 3, "%!#X", 0 );
    TEST( "0x1234abcd", 10, "%!#x", 0x1234abcd );
    TEST( "0x1234ABCD", 10, "%!#X", 0x1234abcd );
    
    TEST( "1101", 4, "%!b", 13 );
    TEST( "1234", 4, "%!o", 01234 );
    TEST( "1234", 4, "%!u", 1234 );
    TEST( "1234abcd", 8, "%!x", 0x1234abcd );
    TEST( "1234ABCD", 8, "%!X", 0x1234ABCD );
    
    TEST( "  0b1101", 8, "%#8b", 13 );
    TEST( "   01234", 8, "%#8o", 01234 );
    TEST( "  0x1234abcd", 12, "%#12x", 0x1234abcd );
    TEST( "  0X1234ABCD", 12, "%#12X", 0x1234abcd );
    
    TEST( "0b00001101", 10, "%#.8b", 13 );
    TEST( "000001234", 9, "%#.8o", 01234 );
    TEST( "0x00001234abcd", 14, "%#.12x", 0x1234abcd );
    TEST( "0X00001234ABCD", 14, "%#.12X", 0x1234abcd );
    
    TEST( "  0b00001101", 12, "%#12.8b", 13 );
    TEST( "   000001234", 12, "%#12.8o", 01234 );
    TEST( "  0x00001234abcd", 16, "%#16.12x", 0x1234abcd );
    TEST( "  0X00001234ABCD", 16, "%#16.12X", 0x1234abcd );
    
    TEST( "0b00001101  ", 12, "%-#12.8b", 13 );
    TEST( "000001234   ", 12, "%-#12.8o", 01234 );
    TEST( "0x00001234abcd  ", 16, "%-#16.12x", 0x1234abcd );
    TEST( "0X00001234ABCD  ", 16, "%-#16.12X", 0x1234abcd );
    
    /* No effect: +,space */
    TEST( "1101", 4, "%+ b", 13 );
    TEST( "1234", 4, "%+ o", 01234 );
    TEST( "1234abcd", 8, "%+ x", 0x1234abcd );
    TEST( "1234ABCD", 8, "%+ X", 0x1234abcd );
}

/*****************************************************************************/
/**
    Test asterisk.
**/
static void test_asterisk( void )
{
    printf( "Testing \"*\"\n" );
    
    /* Precision sets minimum number of digits, zero-padding if necessary */
    TEST( "001234", 6, "%.*d", 6, 1234 );
    
    /* Width sets minimum field width */
    TEST( "  1234", 6, "%*d", 6, 1234 );
    
    /* Both together */
    TEST( "  001234", 8, "%*.*d", 8, 6, 1234 );
}

/*****************************************************************************/
/**
    Test format continuation.
**/
static void test_cont( void )
{
	printf( "Testing format continuation\n" );
	
	/* Basic string continuation */
	TEST( "hello world", 11, "hello %", "world" );
	TEST( "hello old world", 15, "hello %", "old %", "world" );
	
	/* Interspersed conversions */
	TEST( "One: 1,Two: 2,Three: 3", 22, "One: %d,%", 1, 
	                                    "Two: %c,%", '2', 
	                                    "Three: %s", "3" );

	/* Check that flags, precision, width and length are ignored */
    TEST( "hello world", 11, "hello % +-!#12.24l", "world" );
}

/*****************************************************************************/
/**
    Run all tests on format library.
**/
static void run_tests( void )
{
    test();
    test_pc();
    test_c();
    test_n();
    test_s();
    test_p();
    test_di();
    test_bouxX();
    test_asterisk();
    test_cont();
    
    printf( "-----------------------\n"
            "Overall: %s\n", f ? "FAIL" : "PASS" );
}

/*****************************************************************************/
/* Public functions.                                                         */
/*****************************************************************************/

int main( int argc, char *argv[] )
{
    printf( ":: format test harness ::\n");
    run_tests();
    return 0;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
