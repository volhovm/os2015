build: lib/libhelpers.so cat/cat revwords/revwords filter/filter

lib/libhelpers.so cat/cat revwords/revwords filter/filter:
	$(MAKE) -C $(dir $@)

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C cat clean
	$(MAKE) -C revwords clean
	$(MAKE) -C filter clean


rebuild: clean build
