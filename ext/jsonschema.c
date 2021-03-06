/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) moxie & laruence                                       |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: moxie       <system128@gmail.com>                            |
  |         laruence    <laruence@gmail.com>                             |
  +----------------------------------------------------------------------+
  $Id$
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_array.h"
#include "ext/standard/url.h"
//#include "ext/date/php_date.h"
#include "php_jsonschema.h"

#include "ext/standard/html.h"

#if HAVE_PCRE || HAVE_BUNDLED_PCRE
#include "ext/pcre/php_pcre.h"

/* Patterns */
#define PCRE_PATTERN_DATE_ISO8601   "/^([\\+-]?\\d{4}(?!\\d{2}\\b))((-?)((0[1-9]|1[0-2])(\\3([12]\\d|0[1-9]|3[01]))?|W([0-4]\\d|5[0-2])(-?[1-7])?|(00[1-9]|0[1-9]\\d|[12]\\d{2}|3([0-5]\\d|6[1-6])))([T\\s]((([01]\\d|2[0-3])((:?)[0-5]\\d)?|24\\:?00)([\\.,]\\d+(?!:))?)?(\\17[0-5]\\d([\\.,]\\d+)?)?([zZ]|([\\+-])([01]\\d|2[0-3]):?([0-5]\\d)?)?)?)?$/"
#define PCRE_PATTERN_DATE_RU        "/^(0[1-9]|[12][0-9]|3[01])\\.(0[1-9]|1[012])\\.((19|20)\\d\\d)$/"
#define PCRE_PATTERN_DATETIME_RU    "/^(0[1-9]|[12][0-9]|3[01])\\.(0[1-9]|1[012])\\.((19|20)\\d\\d)\\s{1}([01]?[0-9]|2[0-3]):[0-5]0[0-9]:[0-5]0[0-9]$/"
#define PCRE_PATTERN_PHONE_RU       "/^(\+7|8)(?:[-()]*\d){10}$/"
#define PCRE_PATTERN_GUID           "/^[a-z0-9]{8}-[a-z0-9]{4}-[a-z0-9]{4}-[a-z0-9]{4}-[a-z0-9]{12}$/i"
#define PCRE_PATTERN_EMAIL          "/^(?!(?:(?:\\x22?\\x5C[\\x00-\\x7E]\\x22?)|(?:\\x22?[^\\x5C\\x22]\\x22?)){255,})(?!(?:(?:\\x22?\\x5C[\\x00-\\x7E]\\x22?)|(?:\\x22?[^\\x5C\\x22]\\x22?)){65,}@)(?:(?:[\\x21\\x23-\\x27\\x2A\\x2B\\x2D\\x2F-\\x39\\x3D\\x3F\\x5E-\\x7E]+)|(?:\\x22(?:[\\x01-\\x08\\x0B\\x0C\\x0E-\\x1F\\x21\\x23-\\x5B\\x5D-\\x7F]|(?:\\x5C[\\x00-\\x7F]))*\\x22))(?:\\.(?:(?:[\\x21\\x23-\\x27\\x2A\\x2B\\x2D\\x2F-\\x39\\x3D\\x3F\\x5E-\\x7E]+)|(?:\\x22(?:[\\x01-\\x08\\x0B\\x0C\\x0E-\\x1F\\x21\\x23-\\x5B\\x5D-\\x7F]|(?:\\x5C[\\x00-\\x7F]))*\\x22)))*@(?:(?:(?!.*[^.]{64,})(?:(?:(?:xn--)?[a-z0-9]+(?:-+[a-z0-9]+)*\\.){1,126}){1,}(?:(?:[a-z][a-z0-9]*)|(?:(?:xn--)[a-z0-9]+))(?:-+[a-z0-9]+)*)|(?:\\[(?:(?:IPv6:(?:(?:[a-f0-9]{1,4}(?::[a-f0-9]{1,4}){7})|(?:(?!(?:.*[a-f0-9][:\\]]){7,})(?:[a-f0-9]{1,4}(?::[a-f0-9]{1,4}){0,5})?::(?:[a-f0-9]{1,4}(?::[a-f0-9]{1,4}){0,5})?)))|(?:(?:IPv6:(?:(?:[a-f0-9]{1,4}(?::[a-f0-9]{1,4}){5}:)|(?:(?!(?:.*[a-f0-9]:){5,})(?:[a-f0-9]{1,4}(?::[a-f0-9]{1,4}){0,3})?::(?:[a-f0-9]{1,4}(?::[a-f0-9]{1,4}){0,3}:)?)))?(?:(?:25[0-5])|(?:2[0-4][0-9])|(?:1[0-9]{2})|(?:[1-9]?[0-9]))(?:\\.(?:(?:25[0-5])|(?:2[0-4][0-9])|(?:1[0-9]{2})|(?:[1-9]?[0-9]))){3}))\\]))$/iD"
/*
#define PCRE_PATTERN_URI            "#^[a-z0-9:/;,\-_\?&\.%\+\|\#=]*$#i"
*/

#endif

#include "ext/standard/php_var.h"

#define PHP_JSONSCHEMA_CNAME "JsonSchema"
/* private $errors array */
#define ERRORS_PRO "errors"
/* private $complextypes array type schema */
#define JSON_PRO "json"
/* private $complextypes array type schema */
#define COMPLEX_TYPES_PRO "complex_types"



/* If you declare any globals in php_jsonschema.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(jsonschema)
*/

/* True global resources - no need for thread safety here */
/* static int le_jsonschema; */

zend_class_entry * jsonschema_ce;

/** {{{ __construct args
 */
ZEND_BEGIN_ARG_INFO(jsonschema_construct_args, 0)
    ZEND_ARG_PASS_INFO(0)
ZEND_END_ARG_INFO()
/* }}} */

/*{{{ addError args */
ZEND_BEGIN_ARG_INFO(jsonschema_addError_args, 0)
    ZEND_ARG_INFO(0, msg)
ZEND_END_ARG_INFO()
/*}}}*/

/*{{{ addType args */
ZEND_BEGIN_ARG_INFO(jsonschema_addType_args, 0)
    ZEND_ARG_INFO(0, type_schema)
ZEND_END_ARG_INFO()
/*}}}*/

/*{{{ validate args */
ZEND_BEGIN_ARG_INFO(jsonschema_validate_args, 0)
    ZEND_ARG_INFO(0, schema)
ZEND_END_ARG_INFO()
/*}}}*/

/** {{{ static void add_error(HashTable * errors_table TSRMLS_DC, char * format, ...)
 */
static void add_error(HashTable * errors_table TSRMLS_DC, char * format, ...) {
    char  * msg  = NULL;
    char  * html = NULL;
    zval  * temp  = NULL;
    uint msg_len = 0;
    uint new_len = 0;
    va_list args;

    va_start(args, format);
    msg_len = vspprintf(&msg, 0, format, args);
    va_end(args);

    html = php_escape_html_entities_ex(msg, msg_len, &new_len, 0, ENT_COMPAT, NULL, 1 TSRMLS_CC);

    ALLOC_INIT_ZVAL(temp);
    ZVAL_STRINGL(temp, html, new_len, 1);
    zend_hash_next_index_insert(errors_table, (void *)&temp, sizeof (zval *), NULL);

    efree(msg);
    efree(html);
}
/* }}} */

static int parse_hex(const char *str, unsigned int str_len, long *ret TSRMLS_DC) { /* {{{ */
	unsigned long ctx_value = 0;
	const char *end = str + str_len;
	unsigned long n;

	while (str < end) {
		if (*str >= '0' && *str <= '9') {
			n = ((*(str++)) - '0');
		} else if (*str >= 'a' && *str <= 'f') {
			n = ((*(str++)) - ('a' - 10));
		} else if (*str >= 'A' && *str <= 'F') {
			n = ((*(str++)) - ('A' - 10));
		} else {
			return -1;
		}
		if ((ctx_value > ((unsigned long)(~(long)0)) / 16) ||
			((ctx_value = ctx_value * 16) > ((unsigned long)(~(long)0)) - n)) {
			return -1;
		}
		ctx_value += n;
	}

	*ret = (long)ctx_value;
	return 1;
}
/* }}} */

static int validate_ipv4(char *str, int str_len, int *ip) /* {{{ */
{
	const char *end = str + str_len;
	int num, m;
	int n = 0;

	while (str < end) {
		int leading_zero;
		if (*str < '0' || *str > '9') {
			return 0;
		}
		leading_zero = (*str == '0');
		m = 1;
		num = ((*(str++)) - '0');
		while (str < end && (*str >= '0' && *str <= '9')) {
			num = num * 10 + ((*(str++)) - '0');
			if (num > 255 || ++m > 3) {
				return 0;
			}
		}
		/* don't allow a leading 0; that introduces octal numbers,
		 * which we don't support */
		if (leading_zero && (num != 0 || m > 1))
			return 0;
		ip[n++] = num;
		if (n == 4) {
			return str == end;
		} else if (str >= end || *(str++) != '.') {
			return 0;
		}
	}
	return 0;		
}

/* }}} */

static int validate_ipv6(char *str, int str_len TSRMLS_DC) /* {{{ */
{
	int compressed = 0;
	int blocks = 0;
	int n;
	char *ipv4;
	char *end;
	int ip4elm[4];
	char *s = str;

	if (!memchr(str, ':', str_len)) {
		return 0;
	}

	/* check for bundled IPv4 */
	ipv4 = memchr(str, '.', str_len);
	if (ipv4) {
 		while (ipv4 > str && *(ipv4-1) != ':') {
			ipv4--;
		}

		if (!validate_ipv4(ipv4, (str_len - (ipv4 - str)), ip4elm)) {
			return 0;
		}

		str_len = ipv4 - str; /* length excluding ipv4 */
		if (str_len < 2) {
			return 0;
		}

		if (ipv4[-2] != ':') {
			/* don't include : before ipv4 unless it's a :: */
			str_len--;
		}

		blocks = 2;
	}

	end = str + str_len;

	while (str < end) {
		if (*str == ':') {
			if (++str >= end) {
				/* cannot end in : without previous : */
				return 0;
			}
			if (*str == ':') {
				if (compressed) {
					return 0;
				}
				blocks++; /* :: means 1 or more 16-bit 0 blocks */
				compressed = 1;

				if (++str == end) {
					return (blocks <= 8);
				}
			} else if ((str - 1) == s) {
				/* dont allow leading : without another : following */
				return 0;
			}				
		}
		n = 0;
		while ((str < end) &&
		       ((*str >= '0' && *str <= '9') ||
		        (*str >= 'a' && *str <= 'f') ||
		        (*str >= 'A' && *str <= 'F'))) {
			n++;
			str++;
		}
		if (n < 1 || n > 4) {
			return 0;
		}
		if (++blocks > 8)
			return 0;
	}
	return ((compressed && blocks <= 8) || blocks == 8);
}

/* }}} */

static int validate_mac(char *input, int input_len TSRMLS_DC) /* {{{ */
{
	int tokens, length, i, offset, exp_separator_set, exp_separator_len;
	char separator;
	char *exp_separator;
	long ret = 0;
	zval **option_val;

	if (14 == input_len) {
		/* EUI-64 format: Four hexadecimal digits separated by dots. Less
		 * commonly used but valid nonetheless.
		 */
		tokens = 3;
		length = 4;
		separator = '.';
	} else if (17 == input_len && input[2] == '-') {
		/* IEEE 802 format: Six hexadecimal digits separated by hyphens. */
		tokens = 6;
		length = 2;
		separator = '-';
	} else if (17 == input_len && input[2] == ':') {
		/* IEEE 802 format: Six hexadecimal digits separated by colons. */
		tokens = 6;
		length = 2;
		separator = ':';
	} else {
		return 0;
	}

	/* Essentially what we now have is a set of tokens each consisting of
	 * a hexadecimal number followed by a separator character. (With the
	 * exception of the last token which does not have the separator.)
	 */
	for (i = 0; i < tokens; i++) {
		offset = i * (length + 1);

		if (i < tokens - 1 && input[offset + length] != separator) {
			/* The current token did not end with e.g. a "." */
			return 0;
		}
		if (parse_hex(input + offset, length, &ret TSRMLS_CC) < 0) {
			/* The current token is no valid hexadecimal digit */
			return 0;
		}
	}

    return 1;
}
/* }}} */

/** {{{ static zend_bool check_string(HashTable * schema_table, HashTable * errors_table, zval * value TSRMLS_DC)
*/
static zend_bool check_string(HashTable * schema_table, HashTable * errors_table, zval * value TSRMLS_DC) {
    zval ** info = NULL;
    char *  str  = NULL;
    uint    len  = 0;

    long   min_length = 0;
    long   max_length = 0;
    char * format     = NULL;

    zend_bool is_pass = 0;

#if HAVE_PCRE || HAVE_BUNDLED_PCRE
    zval * match   = NULL;
    zval * subpats = NULL;
    char * pattern = NULL;
    pcre_cache_entry * pce_regexp = NULL;
#endif

    do {

        if (value == NULL || Z_TYPE_P(value) != IS_STRING) {
            add_error(errors_table TSRMLS_CC, "value is not a string");
            break;
        }

        str = Z_STRVAL_P(value);
        len = Z_STRLEN_P(value);

        if (zend_hash_find(schema_table, ZEND_STRS("minLength"), (void **) &info) == SUCCESS) {
            if (Z_TYPE_PP(info) != IS_LONG){
                add_error(errors_table TSRMLS_CC, "minLength should be an integer");
                break;
            }
            min_length = Z_LVAL_PP(info);
            if (min_length > len) {
                add_error(errors_table TSRMLS_CC, "value: '%s' is too short", str);
                break;
            }
        }

        if (zend_hash_find(schema_table, ZEND_STRS("maxLength"), (void **) &info) == SUCCESS) {
            if (Z_TYPE_PP(info) != IS_LONG){
                add_error(errors_table TSRMLS_CC, "maxLength should be an integer");
                break;
            }
            max_length = Z_LVAL_PP(info);
            if (max_length < len) {
                add_error(errors_table TSRMLS_CC, "value: '%s' is too long", str);
                break;
            }
        }

        if (zend_hash_find(schema_table, ZEND_STRS("format"), (void **) &info) == SUCCESS) {

            if (Z_TYPE_PP(info) != IS_STRING){
                add_error(errors_table TSRMLS_CC, "max should be an string");
                break;
            }

            format = Z_STRVAL_PP(info);

            /* Internet URL */
            if (strcmp(format, "url") == 0) {

                php_url *url;
                zval * buffer = NULL;

                ALLOC_INIT_ZVAL(buffer);
                ZVAL_STRING(buffer, str, 1);

                /* Use parse_url - if it returns false, we return NULL */
	            url = php_url_parse_ex(Z_STRVAL_P(buffer), Z_STRLEN_P(buffer));
                
	            if (url == NULL) {
                    goto format_bad_url;
	            }

                if (url->scheme != NULL && (!strcasecmp(url->scheme, "http") || !strcasecmp(url->scheme, "https"))) {
            		char *e, *s;

            		if (url->host == NULL) goto format_bad_url;

            		e = url->host + strlen(url->host);
            		s = url->host;

            		/* First char of hostname must be alphanumeric */
            		if(!isalnum((int)*(unsigned char *)s)) goto format_bad_url;

            		while (s < e) {
            			if (!isalnum((int)*(unsigned char *)s) && *s != '-' && *s != '.') {
            				goto format_bad_url;
            			}
            			s++;
                    }
                }

                if (url->scheme == NULL || (url->host == NULL && (strcmp(url->scheme, "mailto") && strcmp(url->scheme, "news") && strcmp(url->scheme, "file")))) {
            format_bad_url:
            		php_url_free(url);
                    add_error(errors_table TSRMLS_CC, "'%s' does not match url format", str);
            		break;
            	}

                php_url_free(url);
                is_pass=1;

                break;
            }

            /* A host name */
            if (strcmp(format, "hostname") == 0 || strcmp(format, "host-name") == 0) {
                php_url *url;
                zval * buffer = NULL;
                char * bufferUrl = NULL;
                char *e, *s;

                /* Let's make a fake http url */
                spprintf(&bufferUrl, 0, "http://%s/", str);

                ALLOC_INIT_ZVAL(buffer);
                ZVAL_STRING(buffer, bufferUrl, 1);

                /* Use parse_url - if it returns false, we return NULL */
	            url = php_url_parse_ex(Z_STRVAL_P(buffer), Z_STRLEN_P(buffer));
                
	            if (url == NULL) goto format_bad_hostname;
                if (url->host == NULL)goto format_bad_hostname;

        		e = url->host + strlen(url->host);
        		s = url->host;

        		/* First char of hostname must be alphanumeric */
        		if(!isalnum((int)*(unsigned char *)s)) goto format_bad_hostname;

        		while (s < e) {
        			if (!isalnum((int)*(unsigned char *)s) && *s != '-' && *s != '.') {
        				goto format_bad_hostname;
        			}
        			s++;
                }

                add_error(errors_table TSRMLS_CC, "'%s'", bufferUrl);

                if (!(url->host != NULL && strcmp(url->host, str) == 0 && strcmp(url->path, "/") == 0 && url->user == NULL && url->pass == NULL && url->port == NULL && url->query == NULL && url->fragment == NULL))
                {
            format_bad_hostname:
                    efree(buffer);
            		php_url_free(url);
                    add_error(errors_table TSRMLS_CC, "'%s' does not match host name format", str);
            		break;
            	}

                php_url_free(url);
                efree(buffer);
                is_pass=1;

                break;
            }

            /* An ip address of version 4 or 6 */
            if (strcmp(format, "ip") == 0 || strcmp(format, "ip-address") == 0) {

                int ip[4];                

                if (strchr (str, ':')) {
            	   is_pass = validate_ipv6(str, len);
            	} else if (strchr (str, '.')) {
            		is_pass = validate_ipv4(str, len, ip);
            	} else {
            		add_error(errors_table TSRMLS_CC, "'%s' does not match IP address format", str);
            	}

                if(is_pass == 0) {
                    add_error(errors_table TSRMLS_CC, "'%s' does not match IP address format", str);
                }

                break;
            }

            /* An ip version 4 address */
            if (strcmp(format, "ipv4") == 0 || strcmp(format, "ipv4-address") == 0) {

                int ip[4];
                is_pass = validate_ipv4(str, len, ip);

                if(is_pass == 0) {
                    add_error(errors_table TSRMLS_CC, "'%s' does not match IPv4 address format", str);
                }

                break;
            }

            /* An ip version 6 address */
            if (strcmp(format, "ipv6") == 0 || strcmp(format, "ipv6-address") == 0) {
                
                is_pass = validate_ipv6(str, len);

                if(is_pass == 0) {
                    add_error(errors_table TSRMLS_CC, "'%s' does not match IPv6 address format", str);
                }

                break;
            }

            /* A MAC address */
            if (strcmp(format, "mac") == 0 || strcmp(format, "mac-address") == 0) {
                
                is_pass = validate_mac(str, len);

                if(is_pass == 0) {
                    add_error(errors_table TSRMLS_CC, "'%s' does not match MAC address format", str);
                }

                break;
            }

#if HAVE_PCRE || HAVE_BUNDLED_PCRE

            match   = NULL;
            subpats = NULL;
            pce_regexp = NULL;

            ALLOC_INIT_ZVAL(match);
            ALLOC_INIT_ZVAL(subpats);

            ZVAL_LONG(match, 0);
            ZVAL_NULL(subpats);

            /* A regular expression, following the regular expression specification from ECMA 262/Perl 5 */
            if (strcmp(format, "regex") == 0) {

                if (zend_hash_find(schema_table, ZEND_STRS("pattern"), (void **) &info) == SUCCESS) {

                    pattern = Z_STRVAL_PP(info);

                    if ((pce_regexp = pcre_get_compiled_regex_cache(pattern,strlen(pattern) TSRMLS_CC)) == NULL) {
                            add_error(errors_table TSRMLS_CC, "'%s' wrong regex patern", pattern);
                        break;
                    }

                    php_pcre_match_impl(pce_regexp, str, len, match, subpats, 0, 0, 0, 0 TSRMLS_CC);

                    if (Z_LVAL_P(match) < 1) {
                        add_error(errors_table TSRMLS_CC, "'%s' does not match regex pattern", str);
                    } else {
                        is_pass = 1;
                    }

                    zval_dtor(match);
                    FREE_ZVAL(match);
                    zval_dtor(subpats);
                    FREE_ZVAL(subpats);
                } else {
                    add_error(errors_table TSRMLS_CC, "format-regex: pattern is undefined");
                }

                break;
            }

            /* GUID */
            if (strcmp(format, "guid") == 0) {

                if ((pce_regexp = pcre_get_compiled_regex_cache(PCRE_PATTERN_GUID, strlen(PCRE_PATTERN_GUID) TSRMLS_CC)) == NULL) {
                    break;
                }

                php_pcre_match_impl(pce_regexp, str, len, match, subpats, 0, 0, 0, 0 TSRMLS_CC);

                if (Z_LVAL_P(match) < 1) {
                    add_error(errors_table TSRMLS_CC, "'%s' does not match GUID format", str);
                } else {
                    is_pass = 1;
                }

                zval_dtor(match);
                FREE_ZVAL(match);
                zval_dtor(subpats);
                FREE_ZVAL(subpats);

                break;
            }

            /* Email address */
            if (strcmp(format, "email") == 0) {
            
                /* The maximum length of an e-mail address is 320 octets, per RFC 2821. */
                if (len > 320) {
                    add_error(errors_table TSRMLS_CC, "'%s' does not match email format", str);
                }

                if ((pce_regexp = pcre_get_compiled_regex_cache(PCRE_PATTERN_EMAIL, strlen(PCRE_PATTERN_EMAIL) TSRMLS_CC)) == NULL) {
                    break;
                }

                php_pcre_match_impl(pce_regexp, str, len, match, subpats, 0, 0, 0, 0 TSRMLS_CC);

                if (Z_LVAL_P(match) < 1) {
                    add_error(errors_table TSRMLS_CC, "'%s' does not match email format", str);
                } else {
                    is_pass = 1;
                }

                zval_dtor(match);
                FREE_ZVAL(match);
                zval_dtor(subpats);
                FREE_ZVAL(subpats);

                break;
            }

            /* A phone format that is used in Russia */
            if (strcmp(format, "phone-ru") == 0) {

                if ((pce_regexp = pcre_get_compiled_regex_cache(PCRE_PATTERN_PHONE_RU, strlen(PCRE_PATTERN_PHONE_RU) TSRMLS_CC)) == NULL) {
                    break;
                }

                php_pcre_match_impl(pce_regexp, str, len, match, subpats, 0, 0, 0, 0 TSRMLS_CC);

                if (Z_LVAL_P(match) < 1) {
                    add_error(errors_table TSRMLS_CC, "'%s' does not match phone format", str);
                } else {
                    is_pass = 1;
                }

                zval_dtor(match);
                FREE_ZVAL(match);
                zval_dtor(subpats);
                FREE_ZVAL(subpats);

                break;
            }

            /* A date in ISO 8601 format */
            if (strcmp(format, "date-time") == 0) {

                zval ** date_format_user = NULL;

                zval *date_value = NULL;
                zval *date_format = NULL;
                zval *retval = NULL;

                ALLOC_INIT_ZVAL(date_value);
                ZVAL_STRING(date_value, str, 0);

                /* Let's check for user defined format */
                if (zend_hash_find(schema_table, ZEND_STRS("format-date-time"), (void **) &date_format_user) == SUCCESS) {

                    if (Z_TYPE_PP(date_format_user) != IS_STRING){
                        add_error(errors_table TSRMLS_CC, "format-date-time should be an string");
                        break;
                    }

                    date_format = &Z_STRVAL_PP(date_format_user);
                }

                /* Use default format if no user defined format was found */
                if(date_format == NULL)
                {
                    ALLOC_INIT_ZVAL(date_format);
                    ZVAL_STRING(date_format, "Y-m-d\\TH:i:sO", 0);
                }

                zend_call_method(NULL, NULL, NULL, "date_parse_from_format", strlen("date_parse_from_format"), &retval, 2, date_format, date_value TSRMLS_CC);

                if (retval == NULL || Z_TYPE_P(retval) != IS_ARRAY) 
                {
                    add_error(errors_table TSRMLS_CC, "'%s' does not match date-time format", str);
                    zval_dtor(retval);
                    break;
                }

                HashTable * result_table = Z_ARRVAL_P(retval);
                zval ** errors = NULL;

                if (zend_hash_find(result_table, ZEND_STRS("error_count"), (void **) &errors) == FAILURE) {
                    add_error(errors_table TSRMLS_CC, "'%s' does not match date-time format", str);
                    zval_dtor(retval);
                    break;
                }

                if (Z_TYPE_PP(errors) != IS_LONG) {
                    add_error(errors_table TSRMLS_CC, "'%s' does not match date-time format", str);
                    zval_dtor(retval);
                    break;
                }

                if(Z_LVAL_PP(errors) > 0) {
                    add_error(errors_table TSRMLS_CC, "'%s' does not match date-time format", str);
                    zval_dtor(retval);
                    break;
                }

                zval_dtor(retval);
                is_pass = 1;

                break;
            }


            /* URI */ 
            /*
            if (strcmp(format, "uri") == 0) {
                if ((pce_regexp = pcre_get_compiled_regex_cache(PCRE_PATTERN_URI, strlen(PCRE_PATTERN_URI) TSRMLS_CC)) == NULL) {
                    break;
                }

                php_pcre_match_impl(pce_regexp, str, len, match, subpats, 0, 0, 0, 0 TSRMLS_CC);

                if (Z_LVAL_P(match) < 1) {
                    add_error(errors_table TSRMLS_CC, "'%s' does not match url format", str);
                } else {
                    is_pass = 1;
                }

                zval_dtor(match);
                FREE_ZVAL(match);
                zval_dtor(subpats);
                FREE_ZVAL(subpats);


                break;
            }
            */
            add_error(errors_table TSRMLS_CC, "format: '%s' is undefined", format);
#else
            add_error(errors_table TSRMLS_CC, "No pcre support to process format property");
#endif
            break;
        }

        is_pass = 1;
    } while (0);

    return is_pass;
}
/* }}} */

/*{{{ get_num */
static zend_bool get_num(zval ** info,double * num) {
    zend_bool is_num = 0;
    zend_uchar type = IS_NULL;
    do{
        if (!info){
            is_num = 0;
            break;
        }

        type = Z_TYPE_PP(info);
        if (type == IS_LONG){
            *num = (double)Z_LVAL_PP(info);
            is_num = 1;
            break;
        }

        if (type == IS_DOUBLE) {
            *num = Z_DVAL_PP(info);
            is_num = 1;
            break;
        }

        is_num = 0;
    }while(0);
    
    return is_num;
}
/*}}}*/

/** {{{ static zend_bool check_number(HashTable * schema_table, HashTable * errors, zval * value TSRMLS_DC)
 */
static zend_bool check_number(HashTable * schema_table, HashTable * errors_table, zval * value TSRMLS_DC) {
    zend_bool is_pass   = 0;
    zval ** info = NULL;
    double val   = 0;
    double num   = 0;

    do {

        if (!get_num(&value, &val)) {
            add_error(errors_table TSRMLS_CC, "value is not a number");
            break;
        }

        if (zend_hash_find(schema_table, ZEND_STRS("minimum"), (void **) &info) == SUCCESS) {

            if (!get_num(info, &num)){
                add_error(errors_table TSRMLS_CC, "minimum should be an integer or double");
                break;
            }
            if (num > val) {
                add_error(errors_table TSRMLS_CC, "value: %f is less than %f", val, num);
                break;
            }
        }

        if (zend_hash_find(schema_table, ZEND_STRS("maximum"), (void **) &info) == SUCCESS) {
            if (!get_num(info, &num)){
                add_error(errors_table TSRMLS_CC, "maximum should be an integer or double", val, num);
                break;
            }
            if (num < val) {
                add_error(errors_table TSRMLS_CC, "value: %f is bigger than %f", val, num);
                break;
            }
        }

        if (zend_hash_find(schema_table, ZEND_STRS("exclusiveMinimum"), (void **) &info) == SUCCESS) {
            if (!get_num(info, &num)){
                add_error(errors_table TSRMLS_CC, "exclusiveMinimum should be an integer or double", val, num);
                break;
            }
            if (num >= val) {
                add_error(errors_table TSRMLS_CC, "value: %f is less than %f or equal", val, num);
                break;
            }
        }

        if (zend_hash_find(schema_table, ZEND_STRS("exclusiveMaximum"), (void **) &info) == SUCCESS) {
            if (!get_num(info, &num)){
                add_error(errors_table TSRMLS_CC, "exclusiveMaximum should be an integer or double", val, num);
                break;
            }
            if (num <= val) {
                add_error(errors_table TSRMLS_CC, "value: %f is bigger than %f or equal", val, num);
                break;
            }
        }

        is_pass = 1;

    } while (0);

    return is_pass;
}
/* }}} */

/** {{{  static zend_bool check_integer(HashTable * schema_table, HashTable * errors_table, zval * value TSRMLS_DC)
 */
static zend_bool check_integer(HashTable * schema_table, HashTable * errors_table, zval * value TSRMLS_DC) {
    zend_bool is_pass = 0;
    if (value == NULL || Z_TYPE_P(value) != IS_LONG) {
        add_error(errors_table TSRMLS_CC, "value is not a integer");
    } else {
        is_pass = check_number(schema_table, errors_table, value TSRMLS_CC);
    }
    return is_pass;
}
/* }}} */

/** {{{ static zend_bool check_bool(HashTable * errors_table, zval * value TSRMLS_DC)
 */
static zend_bool check_bool(HashTable * errors_table, zval * value TSRMLS_DC) {
    zend_bool is_pass = 0;
    if (value == NULL || Z_TYPE_P(value) != IS_BOOL) {
        add_error(errors_table TSRMLS_CC, "value is not a boolean");
    } else {
        is_pass = 1;
    }
    return is_pass;
}
/* }}} */

/** {{{ static zend_bool check_object(HashTable * complex_schemas_table, HashTable * schema_table, HashTable * errors_table, zval * value TSRMLS_DC)
 */
static zend_bool check_object(HashTable * complex_schemas_table, HashTable * schema_table, HashTable * errors_table, zval * value TSRMLS_DC) {
    zend_bool is_pass = 0;
    zval ** item      = NULL;
    char * key        = NULL;
    uint key_len      = 0;
    ulong idx         = 0;
    zval ** optional  = NULL;

    zval ** schema_properties           = NULL;
    HashTable * schema_properties_table = NULL;
    HashTable * values_prop_table       = NULL;
    zval ** item_schema                 = NULL;
    HashPosition pointer                = NULL;

    zval ** schema_key_properties           = NULL;
    HashTable * schema_key_properties_table = NULL;

    do {

        if (value == NULL || (Z_TYPE_P(value) != IS_OBJECT && Z_TYPE_P(value) != IS_ARRAY)) {
            add_error(errors_table TSRMLS_CC, "value is not an object or array");
            break;
        }

        values_prop_table = HASH_OF(value);

        if (values_prop_table == NULL){
            add_error(errors_table TSRMLS_CC, "value is null");
        }

        if (zend_hash_find(schema_table, ZEND_STRS("properties"), (void **) &schema_properties) == FAILURE) {
            add_error(errors_table TSRMLS_CC, "properties:schema properties is undefined");
            break;
        }

        schema_properties_table = Z_ARRVAL_PP(schema_properties);
        item_schema = NULL;
        is_pass = 1;

        for (zend_hash_internal_pointer_reset_ex(values_prop_table, &pointer);
                zend_hash_get_current_data_ex(values_prop_table, (void **) &item, &pointer) == SUCCESS;
                zend_hash_move_forward_ex(values_prop_table, &pointer)) {

            zend_hash_get_current_key_ex(values_prop_table, &key, &key_len, &idx, 0, &pointer);

            if (zend_hash_find(schema_properties_table, key, key_len, (void **) &item_schema) == FAILURE) {
                is_pass = 0;
                add_error(errors_table TSRMLS_CC, "Additional properties not allowed: %s", key);
                continue;
            }

            if (!php_jsonschema_check_by_type(complex_schemas_table, Z_ARRVAL_PP(item_schema), errors_table, * item TSRMLS_CC)) {
                is_pass = 0;
                continue;
            }

        }

        for (zend_hash_internal_pointer_reset_ex(schema_properties_table, &pointer);
                zend_hash_get_current_data_ex(schema_properties_table, (void **) &item, &pointer) == SUCCESS;
                zend_hash_move_forward_ex(schema_properties_table, &pointer)) {

            zend_hash_get_current_key_ex(schema_properties_table, &key, &key_len, &idx, 0, &pointer);

            if (zend_hash_exists(values_prop_table, key, key_len) == 0) {
                if (zend_hash_find(schema_properties_table, ZEND_STRS(key), (void **) &schema_key_properties) == SUCCESS) {
                    schema_key_properties_table = Z_ARRVAL_PP(schema_key_properties);
                    if (zend_hash_find(schema_key_properties_table, ZEND_STRS("optional"), (void **) &optional) == SUCCESS) {
                        if (Z_BVAL_PP(optional)) {
                            continue;
                        }
                    }
                }
                add_error(errors_table TSRMLS_CC, "Missing required property: %s", key);
                break;
            }

        }
    } while (0);

    return is_pass;
}
/* }}} */

/** {{{ static zend_bool check_array(HashTable * complex_schemas_table, HashTable * schema_table, HashTable * errors_table, HashTable * value TSRMLS_DC)
 */
static zend_bool check_array(HashTable * complex_schemas_table, HashTable * schema_table, HashTable * errors_table, zval * value TSRMLS_DC) {
    zend_bool is_pass    = 0;
    zval ** info  = NULL;
    zval ** item  = NULL;
    int min_items = 0;
    int max_items = 0;
    uint size            = 0;
    HashPosition pointer    = {0};
    zval ** items_schema    = NULL;
    HashTable * value_table = NULL;

    do {
        if (value == NULL || Z_TYPE_P(value) != IS_ARRAY) {
            add_error(errors_table TSRMLS_CC, "value is not an array");
            break;
        }

        value_table = Z_ARRVAL_P(value);
        if (zend_hash_find(schema_table, ZEND_STRS("items"), (void **) &items_schema) == FAILURE) {
            add_error(errors_table TSRMLS_CC, "schema: items schema is undefined");
            break;
        }

        size = zend_hash_num_elements(value_table);
        if (zend_hash_find(schema_table, ZEND_STRS("minItems"), (void **) &info) == SUCCESS) {
            if (Z_TYPE_PP(info) != IS_LONG){
                add_error(errors_table TSRMLS_CC, "minItems should be an integer");
                break;
            }
            min_items = Z_LVAL_PP(info);
            if (min_items > size) {
                add_error(errors_table TSRMLS_CC, "array size: %d is less than %d ", size, min_items);
                break;
            }
        }

        if (zend_hash_find(schema_table, ZEND_STRS("maxItems"), (void **) &info) == SUCCESS) {
            if (Z_TYPE_PP(info) != IS_LONG){
                add_error(errors_table TSRMLS_CC, "maxItems should be an integer");
                break;
            }
            max_items = Z_LVAL_PP(info);
            if (max_items < size) {
                add_error(errors_table TSRMLS_CC, "array size: %d is bigger than %d ", size, max_items);
                break;
            }
        }

        if (Z_TYPE_PP(items_schema) != IS_ARRAY) {
            break;
        }

        is_pass = 1;
        for (zend_hash_internal_pointer_reset_ex(value_table, &pointer);
                zend_hash_get_current_data_ex(value_table, (void **) &item, &pointer) == SUCCESS;
                zend_hash_move_forward_ex(value_table, &pointer)) {

            if (!php_jsonschema_check_by_type(complex_schemas_table, Z_ARRVAL_PP(items_schema), errors_table,* item TSRMLS_CC)) {
                is_pass = 0;
                break;
            }

        }
    } while (0);

    return is_pass;
}
/* }}} */

/** {{{ PHP_JSONSCHEMA_API zval * php_jsonschema_get_schema(zval * value TSRMLS_DC)
 */
PHP_JSONSCHEMA_API zval * php_jsonschema_get_schema(zval * value TSRMLS_DC) {
    zval * schema = NULL;
    zval ** item  = NULL;
    HashTable * json_arr_table = NULL;
    HashTable * json_obj_table = NULL;
    HashPosition pointer       = NULL;
    zval * properties  = NULL;
    char * key   = NULL;
    uint key_len = 0;
    ulong idx    = 0;

    ALLOC_INIT_ZVAL(schema);
    array_init(schema);

    switch (Z_TYPE_P(value)) {
        case IS_BOOL:
            add_assoc_string(schema, "type", "boolean", 1);
            add_assoc_bool(schema, "default", 0);
            break;
        case IS_LONG:
            add_assoc_string(schema, "type", "integer", 1);
            add_assoc_long(schema, "default", 0);
            add_assoc_long(schema, "minimum", 0);
            add_assoc_long(schema, "maximum", INT_MAX);
            add_assoc_long(schema, "exclusiveMinimum", 0);
            add_assoc_long(schema, "exclusiveMaximum", INT_MAX);
            break;
        case IS_DOUBLE:
            add_assoc_string(schema, "type", "number", 1);
            add_assoc_long(schema, "default", 0);
            add_assoc_long(schema, "minimum", 0);
            add_assoc_long(schema, "maximum", INT_MAX);
            add_assoc_long(schema, "exclusiveMinimum", 0);
            add_assoc_long(schema, "exclusiveMaximum", INT_MAX);
            break;
        case IS_STRING:
            add_assoc_string(schema, "type", "string", 1);
            add_assoc_string(schema, "format", "regex", 1);
            add_assoc_string(schema, "pattern", "/^[a-z0-9]+$/i", 1);
            add_assoc_string(schema, "type", "string", 1);
            add_assoc_long(schema, "minLength", 0);
            add_assoc_long(schema, "maxLength", INT_MAX);
            break;
        case IS_ARRAY:
            add_assoc_string(schema, "type", "array", 1);
            add_assoc_long(schema, "minItems", 0);
            add_assoc_long(schema, "maxItems", INT_MAX);
            json_arr_table = Z_ARRVAL_P(value);

            if (zend_hash_num_elements(json_arr_table) > 0) {

                for (zend_hash_internal_pointer_reset_ex(json_arr_table, &pointer);
                        zend_hash_get_current_data_ex(json_arr_table, (void**) &item, &pointer) == SUCCESS;
                        zend_hash_move_forward_ex(json_arr_table, &pointer)) {

                    add_assoc_zval(schema, "items", php_jsonschema_get_schema(* item TSRMLS_CC));
                    break;
                }

            }
            break;
        case IS_OBJECT:
            add_assoc_string(schema, "type", "object", 1);
            json_obj_table = Z_OBJPROP_P(value);

            if (zend_hash_num_elements(json_obj_table) > 0) {

                ALLOC_INIT_ZVAL(properties);
                array_init(properties);

                for (zend_hash_internal_pointer_reset_ex(json_obj_table, &pointer);
                        zend_hash_get_current_data_ex(json_obj_table, (void**) &item, &pointer) == SUCCESS;
                        zend_hash_move_forward_ex(json_obj_table, &pointer)) {

                    zend_hash_get_current_key_ex(json_obj_table, &key, &key_len, &idx, 0, &pointer);
                    add_assoc_zval(properties, key, php_jsonschema_get_schema(*item TSRMLS_CC));
                }
                add_assoc_zval(schema, "properties", properties);

            }
            break;
        case IS_NULL:
            add_assoc_null(schema, "type");
            break;
        default:
            break;
    }

    return schema;
}
/* }}} */

/** {{{ PHP_JSONSCHEMA_API void php_jsonschema_add_type(HashTable * complex_schemas_table, HashTable * type_schema_table TSRMLS_DC)
 */
PHP_JSONSCHEMA_API void php_jsonschema_add_type(HashTable * complex_schemas_table, HashTable * type_schema_table TSRMLS_DC) {

    zval ** type_name = NULL;
    zval * new_type   = NULL;

    if (zend_hash_find(type_schema_table, ZEND_STRS("id"), (void **) &type_name) == SUCCESS) {
        convert_to_string_ex(type_name);

        new_type = NULL;
        ALLOC_INIT_ZVAL(new_type);
        array_init(new_type);

        php_array_merge(Z_ARRVAL_P(new_type), type_schema_table, 1 TSRMLS_CC);

        if (zend_hash_find(complex_schemas_table, Z_STRVAL_PP(type_name), Z_STRLEN_PP(type_name) + 1, (void **) &type_name) == SUCCESS) {
            zend_hash_update(complex_schemas_table, Z_STRVAL_PP(type_name), Z_STRLEN_PP(type_name) + 1, (void *) &new_type, sizeof (zval *), NULL);
        } else {
            zend_hash_add(complex_schemas_table, Z_STRVAL_PP(type_name), Z_STRLEN_PP(type_name) + 1, (void *) &new_type, sizeof (HashTable *), NULL);
        }
    }

}
/* }}} */

/*{{{ schema_ctor */
static void schema_ctor(zval ** dest) {
    zval * value = *dest;

    ALLOC_ZVAL(*dest);
    **dest = *value;
    zval_copy_ctor(*dest);
    INIT_PZVAL(*dest);
}
/*}}}*/


/** {{{ PHP_JSONSCHEMA_API zend_bool php_jsonschema_check_by_type(HashTable * complex_schemas_table, HashTable * schema, HashTable * errors_table, zval * value TSRMLS_DC)
 */
PHP_JSONSCHEMA_API zend_bool php_jsonschema_check_by_type(HashTable * complex_schemas_table, HashTable * schema_table, HashTable * errors_table, zval * value TSRMLS_DC) {
    zend_bool is_pass = 0;
    zval ** type = NULL;
    zval ** item = NULL;
    char *  type_name = NULL;
    zval ** complex_type = NULL;

    HashTable *  types_tables = NULL;
    HashPosition pointer      = NULL;

    HashTable * temp_schema_table = NULL;

    do {

        if (zend_hash_find(schema_table, ZEND_STRS("type"), (void **) &type) == FAILURE) {
            break;
        }

        if (Z_TYPE_PP(type) == IS_ARRAY) {
            types_tables = Z_ARRVAL_PP(type);
            pointer = NULL;
            item = NULL;

            for (zend_hash_internal_pointer_reset_ex(types_tables, &pointer);
                    zend_hash_get_current_data_ex(types_tables, (void **) &item, &pointer) == SUCCESS;
                    zend_hash_move_forward_ex(types_tables, &pointer)) {

                if (Z_TYPE_PP(item) != IS_STRING){
                    add_error(errors_table TSRMLS_CC, "type name should be a string");
                    break;
                }

                temp_schema_table = NULL;
                ALLOC_HASHTABLE(temp_schema_table);
                zend_hash_init(temp_schema_table, zend_hash_num_elements(schema_table), NULL, ZVAL_PTR_DTOR, 1);
                zend_hash_copy(temp_schema_table, schema_table, (copy_ctor_func_t) schema_ctor, NULL, sizeof (zval *));
                zend_hash_update(temp_schema_table, "type", sizeof ("type"), (void *) item, sizeof (zval *), NULL);

                is_pass = php_jsonschema_check_by_type(complex_schemas_table, temp_schema_table, errors_table, value TSRMLS_CC);

                zend_hash_destroy(temp_schema_table);
                FREE_HASHTABLE(temp_schema_table);

                if (is_pass == 1) {
                    break;
                }
            }

            break;
        }

        if (Z_TYPE_PP(type) == IS_STRING) {
            type_name = Z_STRVAL_PP(type);

            if (strcmp(type_name, "boolean") == 0) {
                is_pass = check_bool(errors_table, value TSRMLS_CC);

            } else if (strcmp(type_name, "integer") == 0) {

                is_pass = check_integer(schema_table, errors_table, value  TSRMLS_CC);
            } else if (strcmp(type_name, "number") == 0) {

                is_pass = check_number(schema_table, errors_table, value TSRMLS_CC);
            } else if (strcmp(type_name, "string") == 0) {

                is_pass = check_string(schema_table, errors_table, value TSRMLS_CC);
            } else if (strcmp(type_name, "array") == 0) {

                is_pass = check_array(complex_schemas_table, schema_table, errors_table, value TSRMLS_CC);
            } else if (strcmp(type_name, "object") == 0) {

                is_pass = check_object(complex_schemas_table, schema_table, errors_table, value TSRMLS_CC);
            } else if (strcmp(type_name, "null") == 0) {

                is_pass = (Z_TYPE_P(value) == IS_NULL) ? 1 : 0;
            } else if (strcmp(type_name, "any") == 0) {

                is_pass = 1;
            } else {

                add_error(errors_table TSRMLS_CC, "type: '%s' is undefined", type_name);
            }
        }
    } while (0);

    if (zend_hash_find(schema_table, ZEND_STRS("$ref"), (void **) & type) == SUCCESS) {
        if (zend_hash_find(complex_schemas_table, Z_STRVAL_PP(type),
                    Z_STRLEN_PP(type) + 1, (void **) & complex_type) == SUCCESS) {

            temp_schema_table = NULL;
            ALLOC_HASHTABLE(temp_schema_table);
            zend_hash_init(temp_schema_table, zend_hash_num_elements(Z_ARRVAL_PP(complex_type)), NULL, ZVAL_PTR_DTOR, 1);
            zend_hash_copy(temp_schema_table, Z_ARRVAL_PP(complex_type), (copy_ctor_func_t) schema_ctor, NULL, sizeof (zval *));

            is_pass = php_jsonschema_check_by_type(complex_schemas_table, temp_schema_table, errors_table, value TSRMLS_CC);

            zend_hash_destroy(temp_schema_table);
            FREE_HASHTABLE(temp_schema_table);

        } else {

            if (Z_TYPE_PP(type) == IS_STRING) {
                add_error(errors_table TSRMLS_CC, "type schema:'%s' is undefined", Z_STRVAL_PP(type));
            } else {
                add_error(errors_table TSRMLS_CC, "type schema is undefined");
            }
        }

    }
    return is_pass;
}
/* }}} */

/** {{{ proto JsonSchema::__construct($json)
 */
PHP_METHOD(JsonSchema, __construct) {
    zval * self   = NULL;
    zval * errors = NULL;
    zval * complex_types = NULL;

    self = getThis();

    ALLOC_INIT_ZVAL(errors);
    array_init(errors);

    zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL(ERRORS_PRO), errors TSRMLS_CC);

    ALLOC_INIT_ZVAL(complex_types);
    array_init(complex_types);

    zend_update_property(Z_OBJCE_P(self), self, ZEND_STRL(COMPLEX_TYPES_PRO), complex_types TSRMLS_CC);


    zval_ptr_dtor(&errors);
    zval_ptr_dtor(&complex_types);
}
/* }}} */

/** {{{ proto JsonSchema::getSchema($value)
 */
PHP_METHOD(JsonSchema, getSchema) {
    zval * schema = NULL;
    zval * value  = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
                &value) == FAILURE) {

        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "Expected parameter: $value");
    }

    schema = php_jsonschema_get_schema(value TSRMLS_CC);

    RETURN_ZVAL(schema, 1, 1);

}
/* }}} */

/** {{{ proto JsonSchema::addError($msg)
 */
PHP_METHOD(JsonSchema, addError) {
    zval * self    = NULL;
    char * msg_str = NULL;
    int    msg_len = 0;
    zval * errors  = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                &msg_str, &msg_len) == FAILURE) {

        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "Expected parameter: $schema");

    }

    self = getThis();
    errors = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(ERRORS_PRO), 1 TSRMLS_CC);

    add_error(Z_ARRVAL_P(errors) TSRMLS_CC, msg_str TSRMLS_CC, msg_len TSRMLS_CC);

}
/* }}} */

/** {{{ proto JsonSchema::getErrors()
 */
PHP_METHOD(JsonSchema, getErrors) {
    zval * self    = NULL;
    zval * errors  = NULL;


    self = getThis();
    errors = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(ERRORS_PRO), 1 TSRMLS_CC);

    RETURN_ZVAL(errors, 1, 0);

}
/* }}} */

/** {{{ proto JsonSchema::addType($typeSchema)
*/
PHP_METHOD(JsonSchema, addType) {
    zval * self         = NULL;
    zval * type_schema  = NULL;

    zval * complex_schemas = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &type_schema) == FAILURE) {

        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Expected parameter: $typeSchema");

    }

    if (type_schema == NULL || Z_TYPE_P(type_schema) != IS_ARRAY) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "schema parse error. (PHP 5 >= 5.3.0) see json_last_error(void)");
        return;
    }

    /* $complexTypes */
    self = getThis();

    complex_schemas = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(COMPLEX_TYPES_PRO), 1 TSRMLS_CC);

    php_jsonschema_add_type(Z_ARRVAL_P(complex_schemas), Z_ARRVAL_P(type_schema) TSRMLS_CC);

}
/* }}} */

/** {{{ proto JsonSchema::validate($schema, $value)
 */
PHP_METHOD(JsonSchema, validate) {
    zval * self   = NULL;
    zval * schema = NULL;
    zval * value  = NULL;
    zval * errors = NULL;
    zend_bool is_pass = 0;
    zval * complex_schemas = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "az", &schema, &value) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    if (!schema || Z_TYPE_P(schema) != IS_ARRAY) {
        RETURN_FALSE;
    }

    self = getThis();

    errors = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(ERRORS_PRO), 1 TSRMLS_CC);

    complex_schemas = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL(COMPLEX_TYPES_PRO), 1 TSRMLS_CC);

    is_pass = php_jsonschema_check_by_type(Z_ARRVAL_P(complex_schemas), Z_ARRVAL_P(schema), Z_ARRVAL_P(errors), value TSRMLS_CC);

    RETURN_BOOL(is_pass);

}
/* }}} */

/* {{{ jsonschema_functions[]
 *
 * Every user visible function must have an entry in jsonschema_functions[].
 */
zend_function_entry jsonschema_functions[] = {
    PHP_ME(JsonSchema, __construct, jsonschema_construct_args, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(JsonSchema, getSchema  , NULL                     , ZEND_ACC_PUBLIC)
    PHP_ME(JsonSchema, addType    , jsonschema_addType_args  , ZEND_ACC_PUBLIC)
    PHP_ME(JsonSchema, addError   , jsonschema_addError_args , ZEND_ACC_PROTECTED)
    PHP_ME(JsonSchema, getErrors  , NULL                     , ZEND_ACC_PUBLIC)
    PHP_ME(JsonSchema, validate   , jsonschema_validate_args , ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL} /* Must be the last line in jsonschema_functions[] */
};
/* }}} */

/* {{{ jsonschema_module_entry
 */
zend_module_entry jsonschema_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "jsonschema",
    jsonschema_functions,
    PHP_MINIT(jsonschema),
    PHP_MSHUTDOWN(jsonschema),
    PHP_RINIT(jsonschema), /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(jsonschema), /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(jsonschema),
#if ZEND_MODULE_API_NO >= 20010901
    "0.1", /* Replace with version number for your extension */
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

/*{{{ COMPILE_DL_JSONSCHEMA */
#ifdef COMPILE_DL_JSONSCHEMA
ZEND_GET_MODULE(jsonschema)
#endif
/*}}}*/

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(jsonschema) {
    /* If you have INI entries, uncomment these lines
    REGISTER_INI_ENTRIES();
     */
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, PHP_JSONSCHEMA_CNAME, jsonschema_functions);
    jsonschema_ce = zend_register_internal_class(&ce TSRMLS_CC);

    /* errors */
    zend_declare_property_null(jsonschema_ce, ZEND_STRL(ERRORS_PRO), ZEND_ACC_PRIVATE TSRMLS_CC);
    /* json */
    zend_declare_property_null(jsonschema_ce, ZEND_STRL(JSON_PRO), ZEND_ACC_PRIVATE TSRMLS_CC);
    /* complexTypes */
    zend_declare_property_null(jsonschema_ce, ZEND_STRL(COMPLEX_TYPES_PRO), ZEND_ACC_PRIVATE TSRMLS_CC);

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(jsonschema) {
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(jsonschema) {
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(jsonschema) {
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(jsonschema) {
    php_info_print_table_start();
    php_info_print_table_header(2, "jsonschema support", "enabled");
    php_info_print_table_end();
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

