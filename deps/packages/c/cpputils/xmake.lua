
package("cpputils")
    add_urls("https://github.com/PierreEVEN/CppUtils.git")
    on_install(function (package)
        import("package.tools.xmake").install(package)
    end)
package_end()
