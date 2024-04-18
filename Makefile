all:
	@astyle --quiet --options=astylerc src/*.cpp,*.hpp
	@cmake -Bbuild -H.; cmake --build build -j$(nproc)
	@size build/polaris-client-mavlink

install:
	@cmake -Bbuild -H.
	cmake --build build -j$(nproc)
	@sudo cmake --install build
	@mkdir -p ${HOME}/polaris-client-mavlink/logs
	@if [ -f install.config.toml ]; then \
		cp install.config.toml ${HOME}/polaris-client-mavlink/config.toml; \
	else \
		cp config.toml ${HOME}/polaris-client-mavlink/config.toml; \
	fi

clean:
	@rm -rf build
	@echo "All build artifacts removed"

.PHONY: all install clean
