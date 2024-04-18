INSTALL_DIR=${HOME}/polaris-client-mavlink

all:
	@astyle --quiet --options=astylerc src/*.cpp,*.hpp
	@cmake -Bbuild -H.; cmake --build build -j$(nproc)
	@size build/polaris-client-mavlink

install:
	@cmake -Bbuild -H.
	cmake --build build -j$(nproc)
	@sudo cmake --install build
	@if [ -f install.config.toml ]; then \
		mkdir -p $(INSTALL_DIR) && cp install.config.toml $(INSTALL_DIR)/config.toml; \
	else \
		mkdir -p $(INSTALL_DIR) && cp config.toml $(INSTALL_DIR)/config.toml; \
	fi

clean:
	@rm -rf build
	@echo "All build artifacts removed"

.PHONY: all install clean
