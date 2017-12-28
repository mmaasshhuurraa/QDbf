import qbs.base 1.0

Project {
    property bool buildExample: false

    DynamicLibrary {
        name: "QDbf"

        Group {
            name: "publicHeaders"
            prefix: "include/"
            files:[ "*.h" ]
            qbs.install: true
            qbs.installDir: "include"
        }

        Group {
            name: "sources";
            prefix: "src/"
            files:[ "*.cpp" ]
        }

        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: [ "core" ] }

        cpp.defines: [ "QDBF_LIBRARY" ]
        cpp.positionIndependentCode: true
        cpp.cxxLanguageVersion: "c++11"
        cpp.includePaths: "include"
        cpp.visibility: "minimal"

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: "include"
        }

        Group {
            name: "Install library"
            qbs.install: true
            qbs.installDir: "lib"
            fileTagsFilter: product.type
        }
    }

    SubProject {
        filePath: "example/example.qbs"

        Properties {
            condition: buildExample
        }
    }
}
