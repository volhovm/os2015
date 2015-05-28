build: lib/libhelpers.so lib/libbufio.so cat/cat revwords/revwords \
	lenwords/lenwords filter/filter bufcat/bufcat buffilter/buffilter \
	simplesh/simplesh

lib/libhelpers.so lib/libbufio.so cat/cat revwords/revwords lenwords/lenwords filter/filter bufcat/bufcat buffilter/buffilter simplesh/simplesh:
	$(MAKE) -C $(dir $@)

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C cat clean
	$(MAKE) -C revwords clean
	$(MAKE) -C filter clean
	$(MAKE) -C bufcat clean
	$(MAKE) -C buffilter clean
	$(MAKE) -C lenwords clean
	$(MAKE) -C simplesh clean

rebuild: clean build
