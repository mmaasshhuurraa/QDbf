import qbs.base 1.0

Project {
    CppApplication {
        name: "QDbfExample"

        files: [ "*.h", "*.cpp" ]

        Depends { name: "Qt"; submodules: [ "core", "gui", "widgets" ] }
        Depends { name: "QDbf" }

        Group {
            name: "Install application"
            qbs.install: true
            qbs.installDir: "bin"
            fileTagsFilter: product.type
        }
    }
}
