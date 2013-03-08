
all: help_texts generate_mo json gen_js

languages = en de es sv
base_po_files = $(addsuffix .po,$(languages))
php_po_files = $(addprefix po/php/,$(base_po_files))
js_po_files = $(addprefix po/js/,$(base_po_files))
po_files = $(php_po_files) $(js_po_files)
mo_files = $(patsubst %.po,%.mo,$(po_files))
coffee_files = $(wildcard admin/views/default/_js/*.coffee)
js_files = $(patsubst %.coffee,%.js,$(coffee_files))

VERSION=2.3

generate_mo: $(mo_files)

%.mo: %.po
	msgfmt --output-file $@ --statistics --check --verbose $<

update_po: update_po_files=1
update_po: $(po_files)

$(filter-out %en.po,$(php_po_files)): po/php/bubba.pot
$(filter-out %en.po,$(js_po_files)): po/js/bubba.pot
	$(if $(update_po_files),\
	msgmerge \
		--update \
		--verbose \
		--multi-domain \
		$@ \
		$(dir $@)bubba.pot \
	)

%en.po: %bubba.pot
	msginit \
		--locale=en_US \
		--no-translator \
		--input=$(dir $@)bubba.pot \
		--output-file=$@

update_pot: update_pot_files=1
update_pot: po/php/bubba.pot po/js/bubba.pot

po/php/bubba.pot: $(shell cat po/php/POTFILES)
	$(if $(update_pot_files),\
	./php-xgettext \
		--files-from=po/php/POTFILES \
		--default-domain=bubba \
		--output=$@ \
		--from-code=UTF-8 \
		--package-name=bubba-frontend \
		--package-version=$(VERSION) \
		--msgid-bugs-address=info@excito.com \
		--copyright-holder="Excito Electronics AB" \
	)

po/js/bubba.pot: gen_js
po/js/bubba.pot: $(shell cat po/js/POTFILES)
	$(if $(update_pot_files),\
	xgettext \
		--language=JavaScript \
		--files-from=po/js/POTFILES \
		--default-domain=bubba \
		--output=$@ \
		--from-code=UTF-8 \
		--force-po \
		--package-name=bubba-frontend \
		--package-version=$(VERSION) \
		--msgid-bugs-address=info@excito.com \
		--copyright-holder="Excito Electronics AB" \
	)

help_texts: po4a.stamp

po4a.stamp: $(wildcard admin/views/default/help/en/*)
	po4a \
		--msgid-bugs-address=info@excito.com \
		--copyright-holder="Excito Electronics AB" \
		--package-name=bubba-frontend \
		--package-version=$(VERSION) \
		--master-charset=UTF-8 \
		--localized-charset=UTF-8 \
		--keep=80 \
		--rm-backups \
		po4a.conf
	@touch $@

json: $(patsubst %.po,%.json,$(js_po_files))

po/js/%.json: po/js/%.po
	./po2json \
		--pretty \
		--domain bubba \
		--output $@ \
		--add-assign json_locale_data \
		$^


gen_js: $(js_files)

%.js: %.coffee
	coffee --compile --bare $^

clean:
	rm -f $(js_files)
	rm -f po/php/*.mo po/js/*.mo
	rm -f po/php/en.* po/js/en.*
	rm -f po/js/*.json
	rm -f po/php/*~ po/js/*~
	po4a \
		--msgid-bugs-address=info@excito.com \
		--copyright-holder="Excito Electronics AB" \
		--package-name=bubba-frontend \
		--package-version=$(VERSION) \
		--master-charset=UTF-8 \
		--localized-charset=UTF-8 \
		--keep=80 \
		--rm-backups \
		--rm-translations \
		po4a.conf
	rm -f po4a.stamp

locale_dir=$(DESTDIR)/usr/share/web-admin/admin/locale

install:
	$(foreach ll,$(basename $(notdir $(wildcard po/php/*.mo))),\
		install -m 0644 -D  po/php/$(ll).mo $(locale_dir)/$(ll)/LC_MESSAGES/bubba.mo;\
	)
	$(foreach ll,$(basename $(notdir $(wildcard po/js/*.mo))),\
		install -m 0644 -D  po/js/$(ll).json $(locale_dir)/$(ll)/LC_MESSAGES/bubba.json;\
	)

.PHONY: update_po update_pot help_texts all clean gen_js
