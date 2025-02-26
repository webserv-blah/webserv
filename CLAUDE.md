# Webserv Project Guidelines

## Build Commands
- Build: `make`
- Clean objects: `make clean`  
- Clean everything: `make fclean`
- Rebuild: `make re`
- Debug build: `make DEBUG=1`
- Address sanitizer: `make ADDRESS=1`

## Code Style Guidelines
- **Header Guards**: Use uppercase with underscores (`#ifndef FILE_NAME_HPP`)
- **Naming**:
  - Classes: PascalCase (e.g., `GlobalConfig`)
  - Variables: camelCase with trailing underscore for private members (e.g., `clientList_`)
  - Constants: UPPERCASE_WITH_UNDERSCORES
  - Enums: Prefix `Enum` + PascalCase (e.g., `EnumSessionStatus`)
  - Methods: camelCase
- **Error Handling**: Return status codes for error conditions
- **Class Structure**: Public members first, followed by private members
- **Includes**: 
  - System includes with angle brackets
  - Project includes with double quotes
  - Forward declarations to avoid circular dependencies

## Config File Format
Uses NGINX-like syntax for server configuration. See `config.conf` for examples.