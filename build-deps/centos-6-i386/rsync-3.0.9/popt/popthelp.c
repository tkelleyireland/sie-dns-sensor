/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/** \ingroup popt
 * \file popt/popthelp.c
 */

/* (C) 1998-2002 Red Hat, Inc. -- Licensing details are in the COPYING
   file accompanying popt source distributions, available from 
   ftp://ftp.rpm.org/pub/rpm/dist. */

#include "system.h"

/*#define POPT_WCHAR_HACK*/
#ifdef 	POPT_WCHAR_HACK
#include <wchar.h>			/* for mbsrtowcs */
/*@access mbstate_t @*/
#endif
#include "poptint.h"

/*@access poptContext@*/

/**
 * Display arguments.
 * @param con		context
 * @param foo		(unused)
 * @param key		option(s)
 * @param arg		(unused)
 * @param data		(unused)
 */
static void displayArgs(poptContext con,
		/*@unused@*/ UNUSED(enum poptCallbackReason foo),
		struct poptOption * key, 
		/*@unused@*/ UNUSED(const char * arg), /*@unused@*/ UNUSED(void * data))
	/*@globals fileSystem@*/
	/*@modifies fileSystem@*/
{
    if (key->shortName == '?')
	poptPrintHelp(con, stdout, 0);
    else
	poptPrintUsage(con, stdout, 0);
    exit(0);
}

#ifdef	NOTYET
/*@unchecked@*/
static int show_option_defaults = 0;
#endif

/**
 * Empty table marker to enable displaying popt alias/exec options.
 */
/*@observer@*/ /*@unchecked@*/
struct poptOption poptAliasOptions[] = {
    POPT_TABLEEND
};

/**
 * Auto help table options.
 */
/*@-castfcnptr@*/
/*@observer@*/ /*@unchecked@*/
struct poptOption poptHelpOptions[] = {
  { NULL, '\0', POPT_ARG_CALLBACK, (void *)&displayArgs, '\0', NULL, NULL },
  { "help", '?', 0, NULL, '?', N_("Show this help message"), NULL },
  { "usage", '\0', 0, NULL, 'u', N_("Display brief usage message"), NULL },
    POPT_TABLEEND
} ;

/*@observer@*/ /*@unchecked@*/
static struct poptOption poptHelpOptions2[] = {
/*@-readonlytrans@*/
  { NULL, '\0', POPT_ARG_INTL_DOMAIN, PACKAGE, 0, NULL, NULL},
/*@=readonlytrans@*/
  { NULL, '\0', POPT_ARG_CALLBACK, (void *)&displayArgs, '\0', NULL, NULL },
  { "help", '?', 0, NULL, '?', N_("Show this help message"), NULL },
  { "usage", '\0', 0, NULL, 'u', N_("Display brief usage message"), NULL },
#ifdef	NOTYET
  { "defaults", '\0', POPT_ARG_NONE, &show_option_defaults, 0,
	N_("Display option defaults in message"), NULL },
#endif
    POPT_TABLEEND
} ;

/*@observer@*/ /*@unchecked@*/
struct poptOption * poptHelpOptionsI18N = poptHelpOptions2;
/*@=castfcnptr@*/

/**
 * @param table		option(s)
 */
/*@observer@*/ /*@null@*/ static const char *
getTableTranslationDomain(/*@null@*/ const struct poptOption *table)
	/*@*/
{
    const struct poptOption *opt;

    if (table != NULL)
    for (opt = table; opt->longName || opt->shortName || opt->arg; opt++) {
	if (opt->argInfo == POPT_ARG_INTL_DOMAIN)
	    return opt->arg;
    }
    return NULL;
}

/**
 * @param opt		option(s)
 * @param translation_domain	translation domain
 */
/*@observer@*/ /*@null@*/ static const char *
getArgDescrip(const struct poptOption * opt,
		/*@-paramuse@*/ /* FIX: i18n macros disabled with lclint */
		/*@null@*/ UNUSED(const char * translation_domain))
		/*@=paramuse@*/
	/*@*/
{
    if (!(opt->argInfo & POPT_ARG_MASK)) return NULL;

    if (opt == (poptHelpOptions + 1) || opt == (poptHelpOptions + 2))
	if (opt->argDescrip) return POPT_(opt->argDescrip);

    if (opt->argDescrip) return D_(translation_domain, opt->argDescrip);

    switch (opt->argInfo & POPT_ARG_MASK) {
    /*case POPT_ARG_NONE:	return POPT_("NONE");*/ /* impossible */
#ifdef	DYING
    case POPT_ARG_VAL:		return POPT_("VAL");
#else
    case POPT_ARG_VAL:		return NULL;
#endif
    case POPT_ARG_INT:		return POPT_("INT");
    case POPT_ARG_LONG:		return POPT_("LONG");
    case POPT_ARG_STRING:	return POPT_("STRING");
    case POPT_ARG_FLOAT:	return POPT_("FLOAT");
    case POPT_ARG_DOUBLE:	return POPT_("DOUBLE");
    default:			return POPT_("ARG");
    }
}

/**
 * Display default value for an option.
 * @param lineLength	display positions remaining
 * @param opt		option(s)
 * @param translation_domain	translation domain
 * @return
 */
static /*@only@*/ /*@null@*/ char *
singleOptionDefaultValue(size_t lineLength,
		const struct poptOption * opt,
		/*@-paramuse@*/ /* FIX: i18n macros disabled with lclint */
		/*@null@*/ UNUSED(const char * translation_domain))
		/*@=paramuse@*/
	/*@*/
{
    const char * defstr = D_(translation_domain, "default");
    size_t limit, bufsize = 4*lineLength + 1;
    char * le = malloc(bufsize);
    char * l = le;

    if (le == NULL) return NULL;	/* XXX can't happen */
/*@-boundswrite@*/
    *le++ = '(';
    le += strlcpy(le, defstr, bufsize - 3);
    *le++ = ':';
    *le++ = ' ';
    limit = bufsize - (le - l) - 1; /* -1 for closing paren */
    if (opt->arg)	/* XXX programmer error */
    switch (opt->argInfo & POPT_ARG_MASK) {
    case POPT_ARG_VAL:
    case POPT_ARG_INT:
    {	long aLong = *((int *)opt->arg);
	le += snprintf(le, limit, "%ld", aLong);
    }	break;
    case POPT_ARG_LONG:
    {	long aLong = *((long *)opt->arg);
	le += snprintf(le, limit, "%ld", aLong);
    }	break;
    case POPT_ARG_FLOAT:
    {	double aDouble = *((float *)opt->arg);
	le += snprintf(le, limit, "%g", aDouble);
    }	break;
    case POPT_ARG_DOUBLE:
    {	double aDouble = *((double *)opt->arg);
	le += snprintf(le, limit, "%g", aDouble);
    }	break;
    case POPT_ARG_STRING:
    {	const char * s = *(const char **)opt->arg;
	if (s == NULL) {
	    le += strlcpy(le, "null", limit);
	} else {
	    size_t len;
	    limit -= 2; /* make room for quotes */
	    *le++ = '"';
	    len = strlcpy(le, s, limit);
	    if (len >= limit) {
		le += limit - 3 - 1;
		*le++ = '.'; *le++ = '.'; *le++ = '.';
	    } else
		le += len;
	    *le++ = '"';
	}
    }	break;
    case POPT_ARG_NONE:
    default:
	l = _free(l);
	return NULL;
	/*@notreached@*/ break;
    }
    *le++ = ')';
    *le = '\0';
/*@=boundswrite@*/

    return l;
}

/**
 * Display help text for an option.
 * @param fp		output file handle
 * @param maxLeftCol	largest argument display width
 * @param opt		option(s)
 * @param translation_domain	translation domain
 */
static void singleOptionHelp(FILE * fp, size_t maxLeftCol, 
		const struct poptOption * opt,
		/*@null@*/ UNUSED(const char * translation_domain))
	/*@globals fileSystem @*/
	/*@modifies *fp, fileSystem @*/
{
    size_t indentLength = maxLeftCol + 5;
    size_t lineLength = 79 - indentLength;
    const char * help = D_(translation_domain, opt->descrip);
    const char * argDescrip = getArgDescrip(opt, translation_domain);
    size_t helpLength;
    char * defs = NULL;
    char * left;
    size_t lelen, limit;
    size_t nb = maxLeftCol + 1;
    int displaypad = 0;

    /* Make sure there's more than enough room in target buffer. */
    if (opt->longName)	nb += strlen(opt->longName);
    if (argDescrip)	nb += strlen(argDescrip);

/*@-boundswrite@*/
    left = malloc(nb);
    if (left == NULL) return;	/* XXX can't happen */
    left[0] = '\0';
    left[maxLeftCol] = '\0';

    if (opt->longName && opt->shortName)
	snprintf(left, nb, "-%c, %s%s", opt->shortName,
		((opt->argInfo & POPT_ARGFLAG_ONEDASH) ? "-" : "--"),
		opt->longName);
    else if (opt->shortName != '\0') 
	snprintf(left, nb, "-%c", opt->shortName);
    else if (opt->longName)
	snprintf(left, nb, "%s%s",
		((opt->argInfo & POPT_ARGFLAG_ONEDASH) ? "-" : "--"),
		opt->longName);
    if (!*left) goto out;

    if (argDescrip) {
	char * le = left + strlen(left);

	if (opt->argInfo & POPT_ARGFLAG_OPTIONAL)
	    *le++ = '[';

	/* Choose type of output */
	/*@-branchstate@*/
	if (opt->argInfo & POPT_ARGFLAG_SHOW_DEFAULT) {
	    defs = singleOptionDefaultValue(lineLength, opt, translation_domain);
	    if (defs) {
		size_t bufsize = (help ? strlen(help) : 0) + sizeof " " + strlen(defs);
		char * t = malloc(bufsize);
		if (t) {
		    snprintf(t, bufsize, "%s %s", help ? help : "", defs);
		    defs = _free(defs);
		}
		defs = t;
	    }
	}
	/*@=branchstate@*/

	if (opt->argDescrip == NULL) {
	    switch (opt->argInfo & POPT_ARG_MASK) {
	    case POPT_ARG_NONE:
		break;
	    case POPT_ARG_VAL:
#ifdef	NOTNOW	/* XXX pug ugly nerdy output */
	    {	long aLong = opt->val;
		int ops = (opt->argInfo & POPT_ARGFLAG_LOGICALOPS);
		int negate = (opt->argInfo & POPT_ARGFLAG_NOT);

		/* Don't bother displaying typical values */
		if (!ops && (aLong == 0L || aLong == 1L || aLong == -1L))
		    break;
		*le++ = '[';
		switch (ops) {
		case POPT_ARGFLAG_OR:
		    *le++ = '|';
		    /*@innerbreak@*/ break;
		case POPT_ARGFLAG_AND:
		    *le++ = '&';
		    /*@innerbreak@*/ break;
		case POPT_ARGFLAG_XOR:
		    *le++ = '^';
		    /*@innerbreak@*/ break;
		default:
		    /*@innerbreak@*/ break;
		}
		*le++ = (opt->longName != NULL ? '=' : ' ');
		if (negate) *le++ = '~';
		/*@-formatconst@*/
		limit = nb - (le - left);
		lelen = snprintf(le, limit, (ops ? "0x%lx" : "%ld"), aLong);
		le += lelen >= limit ? limit - 1 : lelen;
		/*@=formatconst@*/
		*le++ = ']';
	    }
#endif
		break;
	    case POPT_ARG_INT:
	    case POPT_ARG_LONG:
	    case POPT_ARG_FLOAT:
	    case POPT_ARG_DOUBLE:
	    case POPT_ARG_STRING:
		*le++ = (opt->longName != NULL ? '=' : ' ');
		limit = nb - (le - left);
		lelen = strlcpy(le, argDescrip, limit);
		le += lelen >= limit ? limit - 1 : lelen;
		break;
	    default:
		break;
	    }
	} else {

	    *le++ = '=';
	    limit = nb - (le - left);
	    lelen = strlcpy(le, argDescrip, limit);
	    if (lelen >= limit)
		lelen = limit - 1;
	    le += lelen;

#ifdef	POPT_WCHAR_HACK
	    {	const char * scopy = argDescrip;
		mbstate_t t;
		size_t n;

		memset ((void *)&t, '\0', sizeof (t));	/* In initial state.  */
		/* Determine number of characters.  */
		n = mbsrtowcs (NULL, &scopy, strlen(scopy), &t);

		displaypad = (int) (lelen-n);
	    }
#endif
	}
	if (opt->argInfo & POPT_ARGFLAG_OPTIONAL)
	    *le++ = ']';
	*le = '\0';
    }
/*@=boundswrite@*/

    if (help)
	fprintf(fp,"  %-*s   ", (int)maxLeftCol+displaypad, left);
    else {
	fprintf(fp,"  %s\n", left); 
	goto out;
    }

    left = _free(left);
/*@-branchstate@*/
    if (defs) {
	help = defs;
	defs = NULL;
    }
/*@=branchstate@*/

    helpLength = strlen(help);
/*@-boundsread@*/
    while (helpLength > lineLength) {
	const char * ch;
	char format[16];

	ch = help + lineLength - 1;
	while (ch > help && !isSpace(ch)) ch--;
	if (ch == help) break;		/* give up */
	while (ch > (help + 1) && isSpace(ch)) ch--;
	ch++;

	snprintf(format, sizeof format, "%%.%ds\n%%%ds", (int) (ch - help), (int) indentLength);
	/*@-formatconst@*/
	fprintf(fp, format, help, " ");
	/*@=formatconst@*/
	help = ch;
	while (isSpace(help) && *help) help++;
	helpLength = strlen(help);
    }
/*@=boundsread@*/

    if (helpLength) fprintf(fp, "%s\n", help);

out:
    /*@-dependenttrans@*/
    defs = _free(defs);
    /*@=dependenttrans@*/
    left = _free(left);
}

/**
 * Find display width for longest argument string.
 * @param opt		option(s)
 * @param translation_domain	translation domain
 * @return		display width
 */
static size_t maxArgWidth(const struct poptOption * opt,
		       /*@null@*/ UNUSED(const char * translation_domain))
	/*@*/
{
    size_t max = 0;
    size_t len = 0;
    const char * s;
    
    if (opt != NULL)
    while (opt->longName || opt->shortName || opt->arg) {
	if ((opt->argInfo & POPT_ARG_MASK) == POPT_ARG_INCLUDE_TABLE) {
	    if (opt->arg)	/* XXX program error */
	    len = maxArgWidth(opt->arg, translation_domain);
	    if (len > max) max = len;
	} else if (!(opt->argInfo & POPT_ARGFLAG_DOC_HIDDEN)) {
	    len = sizeof("  ")-1;
	    if (opt->shortName != '\0') len += sizeof("-X")-1;
	    if (opt->shortName != '\0' && opt->longName) len += sizeof(", ")-1;
	    if (opt->longName) {
		len += ((opt->argInfo & POPT_ARGFLAG_ONEDASH)
			? sizeof("-")-1 : sizeof("--")-1);
		len += strlen(opt->longName);
	    }

	    s = getArgDescrip(opt, translation_domain);

#ifdef POPT_WCHAR_HACK
	    /* XXX Calculate no. of display characters. */
	    if (s) {
		const char * scopy = s;
		mbstate_t t;
		size_t n;

/*@-boundswrite@*/
		memset ((void *)&t, '\0', sizeof (t));	/* In initial state.  */
/*@=boundswrite@*/
		/* Determine number of characters.  */
		n = mbsrtowcs (NULL, &scopy, strlen(scopy), &t);
		len += sizeof("=")-1 + n;
	    }
#else
	    if (s)
		len += sizeof("=")-1 + strlen(s);
#endif

	    if (opt->argInfo & POPT_ARGFLAG_OPTIONAL) len += sizeof("[]")-1;
	    if (len > max) max = len;
	}

	opt++;
    }
    
    return max;
}

/**
 * Display popt alias and exec help.
 * @param fp		output file handle
 * @param items		alias/exec array
 * @param nitems	no. of alias/exec entries
 * @param left		largest argument display width
 * @param translation_domain	translation domain
 */
static void itemHelp(FILE * fp,
		/*@null@*/ poptItem items, int nitems, size_t left,
		/*@null@*/ UNUSED(const char * translation_domain))
	/*@globals fileSystem @*/
	/*@modifies *fp, fileSystem @*/
{
    poptItem item;
    int i;

    if (items != NULL)
    for (i = 0, item = items; i < nitems; i++, item++) {
	const struct poptOption * opt;
	opt = &item->option;
	if ((opt->longName || opt->shortName) && 
	    !(opt->argInfo & POPT_ARGFLAG_DOC_HIDDEN))
	    singleOptionHelp(fp, left, opt, translation_domain);
    }
}

/**
 * Display help text for a table of options.
 * @param con		context
 * @param fp		output file handle
 * @param table		option(s)
 * @param left		largest argument display width
 * @param translation_domain	translation domain
 */
static void singleTableHelp(poptContext con, FILE * fp,
		/*@null@*/ const struct poptOption * table, size_t left,
		/*@null@*/ UNUSED(const char * translation_domain))
	/*@globals fileSystem @*/
	/*@modifies *fp, fileSystem @*/
{
    const struct poptOption * opt;
    const char *sub_transdom;

    if (table == poptAliasOptions) {
	itemHelp(fp, con->aliases, con->numAliases, left, NULL);
	itemHelp(fp, con->execs, con->numExecs, left, NULL);
	return;
    }

    if (table != NULL)
    for (opt = table; (opt->longName || opt->shortName || opt->arg); opt++) {
	if ((opt->longName || opt->shortName) && 
	    !(opt->argInfo & POPT_ARGFLAG_DOC_HIDDEN))
	    singleOptionHelp(fp, left, opt, translation_domain);
    }

    if (table != NULL)
    for (opt = table; (opt->longName || opt->shortName || opt->arg); opt++) {
	if ((opt->argInfo & POPT_ARG_MASK) != POPT_ARG_INCLUDE_TABLE)
	    continue;
	sub_transdom = getTableTranslationDomain(opt->arg);
	if (sub_transdom == NULL)
	    sub_transdom = translation_domain;
	    
	if (opt->descrip)
	    fprintf(fp, "\n%s\n", D_(sub_transdom, opt->descrip));

	singleTableHelp(con, fp, opt->arg, left, sub_transdom);
    }
}

/**
 * @param con		context
 * @param fp		output file handle
 */
static int showHelpIntro(poptContext con, FILE * fp)
	/*@globals fileSystem @*/
	/*@modifies *fp, fileSystem @*/
{
    int len = 6;
    const char * fn;

    fprintf(fp, POPT_("Usage:"));
    if (!(con->flags & POPT_CONTEXT_KEEP_FIRST)) {
/*@-boundsread@*/
	/*@-nullderef -type@*/	/* LCL: wazzup? */
	fn = con->optionStack->argv[0];
	/*@=nullderef =type@*/
/*@=boundsread@*/
	if (fn == NULL) return len;
	if (strchr(fn, '/')) fn = strrchr(fn, '/') + 1;
	fprintf(fp, " %s", fn);
	len += strlen(fn) + 1;
    }

    return len;
}

void poptPrintHelp(poptContext con, FILE * fp, /*@unused@*/ UNUSED(int flags))
{
    size_t leftColWidth;

    (void) showHelpIntro(con, fp);
    if (con->otherHelp)
	fprintf(fp, " %s\n", con->otherHelp);
    else
	fprintf(fp, " %s\n", POPT_("[OPTION...]"));

    leftColWidth = maxArgWidth(con->options, NULL);
    singleTableHelp(con, fp, con->options, leftColWidth, NULL);
}

/**
 * Display usage text for an option.
 * @param fp		output file handle
 * @param cursor	current display position
 * @param opt		option(s)
 * @param translation_domain	translation domain
 */
static size_t singleOptionUsage(FILE * fp, size_t cursor, 
		const struct poptOption * opt,
		/*@null@*/ const char *translation_domain)
	/*@globals fileSystem @*/
	/*@modifies *fp, fileSystem @*/
{
    size_t len = 4;
    char shortStr[2] = { '\0', '\0' };
    const char * item = shortStr;
    const char * argDescrip = getArgDescrip(opt, translation_domain);

    if (opt->shortName != '\0' && opt->longName != NULL) {
	len += 2;
	if (!(opt->argInfo & POPT_ARGFLAG_ONEDASH)) len++;
	len += strlen(opt->longName);
    } else if (opt->shortName != '\0') {
	len++;
	shortStr[0] = opt->shortName;
	shortStr[1] = '\0';
    } else if (opt->longName) {
	len += strlen(opt->longName);
	if (!(opt->argInfo & POPT_ARGFLAG_ONEDASH)) len++;
	item = opt->longName;
    }

    if (len == 4) return cursor;

#ifdef POPT_WCHAR_HACK
    /* XXX Calculate no. of display characters. */
    if (argDescrip) {
	const char * scopy = argDescrip;
	mbstate_t t;
	size_t n;

/*@-boundswrite@*/
	memset ((void *)&t, '\0', sizeof (t));	/* In initial state.  */
/*@=boundswrite@*/
	/* Determine number of characters.  */
	n = mbsrtowcs (NULL, &scopy, strlen(scopy), &t);
	len += sizeof("=")-1 + n;
    }
#else
    if (argDescrip) 
	len += sizeof("=")-1 + strlen(argDescrip);
#endif

    if ((cursor + len) > 79) {
	fprintf(fp, "\n       ");
	cursor = 7;
    } 

    if (opt->longName && opt->shortName) {
	fprintf(fp, " [-%c|-%s%s%s%s]",
	    opt->shortName, ((opt->argInfo & POPT_ARGFLAG_ONEDASH) ? "" : "-"),
	    opt->longName,
	    (argDescrip ? " " : ""),
	    (argDescrip ? argDescrip : ""));
    } else {
	fprintf(fp, " [-%s%s%s%s]",
	    ((opt->shortName || (opt->argInfo & POPT_ARGFLAG_ONEDASH)) ? "" : "-"),
	    item,
	    (argDescrip ? (opt->shortName != '\0' ? " " : "=") : ""),
	    (argDescrip ? argDescrip : ""));
    }

    return cursor + len + 1;
}

/**
 * Display popt alias and exec usage.
 * @param fp		output file handle
 * @param cursor	current display position
 * @param item		alias/exec array
 * @param nitems	no. of ara/exec entries
 * @param translation_domain	translation domain
 */
static size_t itemUsage(FILE * fp, size_t cursor,
		/*@null@*/ poptItem item, int nitems,
		/*@null@*/ UNUSED(const char * translation_domain))
	/*@globals fileSystem @*/
	/*@modifies *fp, fileSystem @*/
{
    int i;

    /*@-branchstate@*/		/* FIX: W2DO? */
    if (item != NULL)
    for (i = 0; i < nitems; i++, item++) {
	const struct poptOption * opt;
	opt = &item->option;
        if ((opt->argInfo & POPT_ARG_MASK) == POPT_ARG_INTL_DOMAIN) {
	    translation_domain = (const char *)opt->arg;
	} else if ((opt->longName || opt->shortName) &&
		 !(opt->argInfo & POPT_ARGFLAG_DOC_HIDDEN)) {
	    cursor = singleOptionUsage(fp, cursor, opt, translation_domain);
	}
    }
    /*@=branchstate@*/

    return cursor;
}

/**
 * Keep track of option tables already processed.
 */
typedef struct poptDone_s {
    int nopts;
    int maxopts;
    const void ** opts;
} * poptDone;

/**
 * Display usage text for a table of options.
 * @param con		context
 * @param fp		output file handle
 * @param cursor	current display position
 * @param opt		option(s)
 * @param translation_domain	translation domain
 * @param done		tables already processed
 * @return
 */
static size_t singleTableUsage(poptContext con, FILE * fp, size_t cursor,
		/*@null@*/ const struct poptOption * opt,
		/*@null@*/ UNUSED(const char * translation_domain),
		/*@null@*/ poptDone done)
	/*@globals fileSystem @*/
	/*@modifies *fp, done, fileSystem @*/
{
    /*@-branchstate@*/		/* FIX: W2DO? */
    if (opt != NULL)
    for (; (opt->longName || opt->shortName || opt->arg) ; opt++) {
        if ((opt->argInfo & POPT_ARG_MASK) == POPT_ARG_INTL_DOMAIN) {
	    translation_domain = (const char *)opt->arg;
	} else if ((opt->argInfo & POPT_ARG_MASK) == POPT_ARG_INCLUDE_TABLE) {
	    if (done) {
		int i = 0;
		for (i = 0; i < done->nopts; i++) {
/*@-boundsread@*/
		    const void * that = done->opts[i];
/*@=boundsread@*/
		    if (that == NULL || that != opt->arg)
			/*@innercontinue@*/ continue;
		    /*@innerbreak@*/ break;
		}
		/* Skip if this table has already been processed. */
		if (opt->arg == NULL || i < done->nopts)
		    continue;
/*@-boundswrite@*/
		if (done->nopts < done->maxopts)
		    done->opts[done->nopts++] = (const void *) opt->arg;
/*@=boundswrite@*/
	    }
	    cursor = singleTableUsage(con, fp, cursor, opt->arg,
			translation_domain, done);
	} else if ((opt->longName || opt->shortName) &&
		 !(opt->argInfo & POPT_ARGFLAG_DOC_HIDDEN)) {
	    cursor = singleOptionUsage(fp, cursor, opt, translation_domain);
	}
    }
    /*@=branchstate@*/

    return cursor;
}

/**
 * Return concatenated short options for display.
 * @todo Sub-tables should be recursed.
 * @param opt		option(s)
 * @param fp		output file handle
 * @retval str		concatenation of short options
 * @return		length of display string
 */
static int showShortOptions(const struct poptOption * opt, FILE * fp,
		/*@null@*/ char * str)
	/*@globals fileSystem @*/
	/*@modifies *str, *fp, fileSystem @*/
	/*@requires maxRead(str) >= 0 @*/
{
    /* bufsize larger then the ascii set, lazy alloca on top level call. */
    char * s = (str != NULL ? str : memset(alloca(300), 0, 300));
    int len = 0;

    if (s == NULL)
	return 0;

/*@-boundswrite@*/
    if (opt != NULL)
    for (; (opt->longName || opt->shortName || opt->arg); opt++) {
	if (opt->shortName && !(opt->argInfo & POPT_ARG_MASK))
	    s[strlen(s)] = opt->shortName;
	else if ((opt->argInfo & POPT_ARG_MASK) == POPT_ARG_INCLUDE_TABLE)
	    if (opt->arg)	/* XXX program error */
		len = showShortOptions(opt->arg, fp, s);
    } 
/*@=boundswrite@*/

    /* On return to top level, print the short options, return print length. */
    if (s == str && *s != '\0') {
	fprintf(fp, " [-%s]", s);
	len = strlen(s) + sizeof(" [-]")-1;
    }
    return len;
}

void poptPrintUsage(poptContext con, FILE * fp, /*@unused@*/ UNUSED(int flags))
{
    poptDone done = memset(alloca(sizeof(*done)), 0, sizeof(*done));
    size_t cursor;

    done->nopts = 0;
    done->maxopts = 64;
    cursor = done->maxopts * sizeof(*done->opts);
/*@-boundswrite@*/
    done->opts = memset(alloca(cursor), 0, cursor);
    /*@-keeptrans@*/
    done->opts[done->nopts++] = (const void *) con->options;
    /*@=keeptrans@*/
/*@=boundswrite@*/

    cursor = showHelpIntro(con, fp);
    cursor += showShortOptions(con->options, fp, NULL);
    cursor = singleTableUsage(con, fp, cursor, con->options, NULL, done);
    cursor = itemUsage(fp, cursor, con->aliases, con->numAliases, NULL);
    cursor = itemUsage(fp, cursor, con->execs, con->numExecs, NULL);

    if (con->otherHelp) {
	cursor += strlen(con->otherHelp) + 1;
	if (cursor > 79) fprintf(fp, "\n       ");
	fprintf(fp, " %s", con->otherHelp);
    }

    fprintf(fp, "\n");
}

void poptSetOtherOptionHelp(poptContext con, const char * text)
{
    con->otherHelp = _free(con->otherHelp);
    con->otherHelp = xstrdup(text);
}