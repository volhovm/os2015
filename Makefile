build: lib/libhelpers.so cat/cat revwords/revwords

lib/libhelpers.so cat/cat revwords/revwords:
	$(MAKE) -C $(dir $@)

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C cat clean
	$(MAKE) -C revwords clean


rebuild: clean build
