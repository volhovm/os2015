build: lib/libhelpers.so lib/libbufio.so cat/cat revwords/revwords filter/filter bufcat/bufcat

lib/libhelpers.so lib/libbufio.so cat/cat revwords/revwords filter/filter:
	$(MAKE) -C $(dir $@)

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C cat clean
	$(MAKE) -C revwords clean
	$(MAKE) -C filter clean
	$(MAKE) -C bufcat clean

rebuild: clean build
