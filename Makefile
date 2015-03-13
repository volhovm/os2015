DIRS = lib

build:
	$(MAKE) -C $(DIRS)
clean:
	$(MAKE) -C $(DIRS) clean
rebuild: clean build
