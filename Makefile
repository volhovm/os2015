build: lib/libhelpers.so cat/cat revwords/revwords lenwords/lenwords

lib/libhelpers.so cat/cat revwords/revwords lenwords/lenwords:
	$(MAKE) -C $(dir $@)

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C cat clean
	$(MAKE) -C revwords clean
	$(MAKE) -C lenwords clean


rebuild: clean build
