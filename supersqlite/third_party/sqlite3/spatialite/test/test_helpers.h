#if __GNUC__
#define UNUSED __attribute__ ((__unused__))
#else
#define UNUSED
#endif

#ifdef __WIN32
/*
 * Windows replacement for strcastr
 *
 * original code from:
 * https://code.google.com/p/msysgit/source/browse/compat/strcasestr.c?repo=4msysgit&name=mingw-v1.5.2.4-devel
 */
static char *
strcasestr (const char *haystack, const char *needle)
{
    int nlen = strlen (needle);
    int hlen = strlen (haystack) - nlen + 1;
    int i;

    for (i = 0; i < hlen; i++)
      {
	  int j;
	  for (j = 0; j < nlen; j++)
	    {
		unsigned char c1 = haystack[i + j];
		unsigned char c2 = needle[j];
		if (toupper (c1) != toupper (c2))
		    goto next;
	    }
	  return (char *) haystack + i;
	next:
	  ;
      }
    return NULL;
}
#endif

#ifdef __WIN32
/* 
 * public domain strtok_r() by Charlie Gordon
 *
 *   from comp.lang.c  9/14/2007
 *
 *      http://groups.google.com/group/comp.lang.c/msg/2ab1ecbb86646684
 *
 *     (Declaration that it's public domain):
 *      http://groups.google.com/group/comp.lang.c/msg/7c7b39328fefab9c
 */
static char *
strtok_r (char *str, const char *delim, char **nextp)
{
    char *ret;

    if (str == NULL)
      {
	  str = *nextp;
      }

    str += strspn (str, delim);

    if (*str == '\0')
      {
	  return NULL;
      }

    ret = str;

    str += strcspn (str, delim);

    if (*str)
      {
	  *str++ = '\0';
      }

    *nextp = str;

    return ret;
}
#endif
