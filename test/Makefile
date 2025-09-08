SUBDIRS = $(shell find . -maxdepth 1 -mindepth 1 -type d | sed 's,\./,,g')
QMAKE = qmake

tests:
	$(foreach subdir, $(SUBDIRS), \
		echo "\n[*] Making all in $(subdir)"; \
        cd $(subdir) && $(QMAKE) && $(MAKE) -j$(shell nproc) all || exit 1; \
		./test_$(subdir) || exit 1; cd .. ; \
	)

clean:
	$(foreach subdir, $(SUBDIRS), \
        cd $(subdir) && $(MAKE) --ignore-errors clean && rm -f .qmake.stash; cd .. ; \
	)