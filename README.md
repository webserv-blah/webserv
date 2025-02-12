## Directory Structure
```
├── include_make
    ├── Link.mk
    ├── Recipe_src.mk
    ├── Recipe_subsrc.mk
    ├── Rules.mk
├── src
    ├── include
    ├── {subsrc1}
        ├── .cpp
        ├── .hpp
        └── Makefile
    ├── {subsrc2}
        ├── .cpp
        ├── .hpp
        └── Makefile
    ├── main.cpp
    ├── .cpp
    └── Makefile
└── Makefile
```

## Need to Edit
-  중괄호 안의 내용을 편집하여 사용
1. `NAME := {program}` in `src/Makefile`
2. `NAME := {subsrc}.a`&`HEAD := {subsrc}.hpp` in `src/{subsrc}/Makefile`
	- **{subsrc}는 디렉토리 이름과 일치해야함**
