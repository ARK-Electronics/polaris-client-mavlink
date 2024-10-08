PROJECT_NAME="polaris-client-mavlink"

all:
	@astyle --quiet --options=astylerc src/*.cpp,*.hpp
	@cmake -Bbuild -H.; cmake --build build -j$(nproc)
	@size build/${PROJECT_NAME}

install:
	@bash install.sh

clean:
	@rm -rf build
	@echo "All build artifacts removed"

.PHONY: all install clean
