build: lib/libhelpers.so lib/libbufio.so cat/cat revwords/revwords \
	lenwords/lenwords filter/filter bufcat/bufcat buffilter/buffilter \
	simplesh/simplesh filesender/filesender bipiper/forking

lib/libhelpers.so lib/libbufio.so cat/cat revwords/revwords lenwords/lenwords filter/filter bufcat/bufcat buffilter/buffilter simplesh/simplesh filesender/filesender bipiper/forking:
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
	$(MAKE) -C filesender clean
	$(MAKE) -C bipiper clean

rebuild: clean build
