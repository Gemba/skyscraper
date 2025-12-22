SUBDIRS = $(shell find . -maxdepth 1 -mindepth 1 -type d | sed 's,\./,,g' | sort)
QMAKE = qmake6

tests:
	$(foreach subdir, $(SUBDIRS), \
		echo "\n[*] Making all in test/$(subdir) ..."; \
		cd $(subdir) && $(QMAKE); \
		$(MAKE) -j$(shell nproc) all || exit 1; \
		./test_$(subdir) || exit 1; \
		echo "[*] ... done tests in test/$(subdir).\n"; \
		cd .. ; \
	)

clean:
	$(foreach subdir, $(SUBDIRS), \
		cd $(subdir) && $(MAKE) --ignore-errors distclean; \
		cd .. ; \
	)
