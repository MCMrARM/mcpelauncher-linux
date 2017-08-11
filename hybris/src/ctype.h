#ifndef CTYPE_H_
#define CTYPE_H_

extern const char *_hybris_ctype_;
extern const short *_hybris_tolower_tab_;
extern const short *_hybris_toupper_tab_;

int
hybris_isalnum(int c);

int
hybris_isalpha(int c);

int
hybris_isblank(int c);

int
hybris_iscntrl(int c);

int
hybris_isdigit(int c);

int
hybris_isgraph(int c);

int
hybris_islower(int c);

int
hybris_isprint(int c);

int
hybris_ispunct(int c);

int
hybris_isspace(int c);

int
hybris_isupper(int c);

int
hybris_isxdigit(int c);

#endif