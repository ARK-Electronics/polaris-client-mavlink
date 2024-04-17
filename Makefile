all:
	@astyle --quiet --options=astylerc src/*.cpp,*.hpp
	@cmake -Bbuild -H.; cmake --build build -j$(nproc)
	@size build/polaris-rtk-client

install:
	@cmake -Bbuild -H.
	cmake --build build -j$(nproc)
	@sudo cmake --install build
	@mkdir -p ${HOME}/polaris-rtk-client/logs
	@if [ -f install.config.toml ]; then \
		cp install.config.toml ${HOME}/polaris-rtk-client/config.toml; \
	else \
		cp config.toml ${HOME}/polaris-rtk-client/config.toml; \
	fi

clean:
	@rm -rf build
	@echo "All build artifacts removed"

.PHONY: all install clean
