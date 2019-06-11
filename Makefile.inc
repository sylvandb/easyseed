-include Makefile.local

PROG=easyseed
MANSEC=1
OBJS=	$(PROG).o \
	vectors.o


VECTORSRC=	mkvectors.sh \
		vectors.h \
		vectors.json

WORDLISTS :=
LANG_ENGLISH ?= yes
LANG_INTL ?= yes

ifeq (yes, $(LANG_INTL))
	LANG_CHINESE_SIMPLIFIED := yes
	LANG_CHINESE_TRADITIONAL := yes
	LANG_CZECH := yes
	LANG_ENGLISH := yes
	LANG_FRENCH := yes
	LANG_INDONESIAN := yes
	LANG_ITALIAN := yes
	LANG_JAPANESE := yes
	LANG_KOREAN := yes
	LANG_RUSSIAN := yes
	LANG_SPANISH := yes
	LANG_UKRAINIAN:= yes
endif

ifeq (yes, $(LANG_CHINESE_SIMPLIFIED))
	WORDLISTS += chinese_simplified
	CFLAGS += -DCHINESE_SIMPLIFIED
endif
ifeq (yes, $(LANG_CHINESE_TRADITIONAL))
	WORDLISTS += chinese_traditional
	CFLAGS += -DCHINESE_TRADITIONAL
endif
ifeq (yes, $(LANG_CZECH))
	WORDLISTS += czech
	CFLAGS += -DCZECH
endif
ifeq (yes, $(LANG_ENGLISH))
	WORDLISTS += english
	CFLAGS += -DENGLISH
	VECTORSRC += test_EN_BIP39.json
endif
ifeq (yes, $(LANG_FRENCH))
	WORDLISTS += french
	CFLAGS += -DFRENCH
endif
ifeq (yes, $(LANG_INDONESIAN))
	WORDLISTS += indonesian
	CFLAGS += -DINDONESIAN
endif
ifeq (yes, $(LANG_ITALIAN))
	WORDLISTS += italian
	CFLAGS += -DITALIAN
endif
ifeq (yes, $(LANG_JAPANESE))
	WORDLISTS += japanese
	CFLAGS += -DJAPANESE
	VECTORSRC += test_JP_BIP39.json
endif
ifeq (yes, $(LANG_KOREAN))
	WORDLISTS += korean
	CFLAGS += -DKOREAN
endif
ifeq (yes, $(LANG_RUSSIAN))
	WORDLISTS += russian
	CFLAGS += -DRUSSIAN
endif
ifeq (yes, $(LANG_SPANISH))
	WORDLISTS += spanish
	CFLAGS += -DSPANISH
endif
ifeq (yes, $(LANG_UKRAINIAN))
	WORDLISTS += ukrainian
	CFLAGS += -DUKRAINIAN
endif

libutf8proc.a: utf8proc/utf8proc.[ch] utf8proc/utf8proc_data.c
	make -C utf8proc libutf8proc.a
	mv utf8proc/libutf8proc.a $@

clean-libutf8proc:
	rm -f libutf8proc.a
	make -C utf8proc clean

#manfmt: $(PROG).$(MANSEC).html $(PROG).$(MANSEC).md $(PROG).$(MANSEC).txt
manfmt: $(PROG).$(MANSEC).html $(PROG).$(MANSEC).txt

$(PROG).$(MANSEC).html: $(PROG).$(MANSEC)
	mandoc -I os=Bitcoin -T html $< > $@

#Currently broken with message:
#mandoc -I os=Bitcoin -T markdown easyseed.1 > easyseed.1.md
#mandoc: mdoc_markdown.c:333: md_node: Assertion `n->tok >= MDOC_Dd && n->tok < MDOC_MAX' failed.
#$(PROG).$(MANSEC).md: $(PROG).$(MANSEC)
#	mandoc -I os=Bitcoin -T markdown $< > $@

$(PROG).$(MANSEC).txt: $(PROG).$(MANSEC)
	# XXX: Ridiculous kludge around make/shell quoting issues which
	# prevent me from simply using sed expression $'s/[^\b]\b//g' :
	mandoc -I os=Bitcoin -T utf8 $< | \
		tr '\b' '^' | sed $(SEDRE) -e 's/[^^]\^//g' > $@

wordlist.h: wordlist/*
	for lang in $(WORDLISTS) ; do \
		echo "static const char *$${lang}[2048] = {" ; \
		{ \
			ctr=2048 ; \
			while read w ; do \
				if [ $$((ctr -= 1)) -eq 0 ] ; then \
					printf '\tu8"%s"\n};\n' "$$w" ; \
				else \
					printf '\tu8"%s",\n' "$$w" ; \
				fi ; \
			done < wordlist/$${lang}.txt ; \
			printf 'static const char %s_hash[] = "%s";\n' \
				$${lang} \
					`$(HASHPROG) wordlist/$${lang}.txt | \
						grep -Eo '^[0-9a-f]+'` ; \
		} ; \
	done > $@

#
# Requires jq.  Also, the shell script is awful.
# I will check in a "baked" version.
#
vectors.c: $(VECTORSRC)
	./mkvectors.sh > $@

clean: clean-libutf8proc
	rm -f $(PROG) $(OBJS) wordlist.h

distclean: clean
	rm -f $(PROG).$(MANSEC).html $(PROG).$(MANSEC).md $(PROG).$(MANSEC).txt vectors.c

check: $(PROG)
	./$(PROG) -T
