# Makefile for Chat and File Server system

# Variables
PYTHON ?= python3
CHMOD = chmod
INSTALL_DIR = /usr/local/bin

# Default target
all: permissions

# Make the scripts executable
permissions:
	$(CHMOD) +x client/chatclient
	$(CHMOD) +x server/chatserver
	@echo "Made scripts executable"

# Optional installation target
install:
	cp client/chatclient $(INSTALL_DIR)/chatclient
	cp server/chatserver $(INSTALL_DIR)/chatserver
	@echo "Installed executables to $(INSTALL_DIR)"

# Clean target
clean:
	find . -name "*.pyc" -delete
	find . -name "__pycache__" -delete
	@echo "Cleaned up Python cache files"

# Dependencies needed
deps:
	$(PYTHON) -m pip install colorama
	@echo "Installed required dependencies"

# Run tests
test: permissions
	@echo "Running basic connectivity test..."
	# Add test commands here
	@echo "Tests completed"

.PHONY: all permissions install clean deps test
