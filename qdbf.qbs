import qbs.base 1.0

Project {
    DynamicLibrary {
        name: "QDbf"

        files: [ "include/*.h", "src/*.cpp" ]

        cpp.includePaths: "include"
        cpp.defines: [ "QDBF_LIBRARY" ]
        cpp.cxxLanguageVersion: "c++11"
        cpp.positionIndependentCode: true

        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: [ "core" ] }

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: "include"
        }

        FileTagger {
            patterns: "*.h"
            fileTags: ["publicHeaders"]
        }

        Group {
            name: "Install library"
            qbs.install: true
            qbs.installDir: "lib"
            fileTagsFilter: "dynamiclibrary"
        }

        Group {
            name: "Install headers"
            qbs.install: true
            qbs.installDir: "include"
            fileTagsFilter: "publicHeaders"
        }
    }

    references: [ "example/example.qbs" ]
}
