#include "commands/cmd_init.h"
#include "filesystem.h"

#include <stdio.h>
#include <string.h>

// NOTE: update this to match standard libary convention if and when it changes
static const char *template_main_mach = "use          std.system.runtime;\n"
                                        "use          std.types.string;\n"
                                        "use          std.collections.slice;\n"
                                        "use console: std.io.console;\n"
                                        "\n"
                                        "$main.symbol = \"main\";\n"
                                        "fun main(args: Slice[str]) i64 {\n"
                                        "    console.print(\"Hello, World!\\n\");\n"
                                        "    ret 0;\n"
                                        "}\n";

// NOTE: update this to match config convention if and when it changes
const char *template_mach_toml = "[project]\n"
                                 "id      = \"%s\"\n"
                                 "name    = \"%s\"\n"
                                 "version = \"0.1.0\"\n"
                                 "dir_src = \"src\"\n"
                                 "dir_out = \"out\"\n"
                                 "dir_dep = \"dep\"\n"
                                 "target  = \"native\"\n"
                                 "\n"
                                 "[targets.linux]\n"
                                 "platform   = \"linux\"\n"
                                 "arch       = \"x86_64\"\n"
                                 "mode       = \"executable\"\n"
                                 "entrypoint = \"main.mach\"\n"
                                 "artifacts  = \"linux\"\n"
                                 "binary     = \"linux/bin/%s\"\n"
                                 "\n"
                                 "[dep.mach-std]\n"
                                 "type    = \"remote\"\n"
                                 "path    = \"https://github.com/octalide/mach-std\"\n"
                                 "version = \"branch/main\"\n";

void cmd_init_help(FILE *stream)
{
    fprintf(stream, "usage: mach init <project_id>\n");
    fprintf(stream, "\n");
    fprintf(stream, "create a new Mach project in the current directory with the specified project ID\n");
}

int cmd_init_handle(int argc, char **argv)
{
    if (argc < 3)
    {
        cmd_init_help(stderr);
        return 1;
    }

    char *project_id = argv[2];

    // cap project_id length
    if (strlen(project_id) >= 128)
    {
        fprintf(stderr, "error: project ID too long (max 127 characters)\n");
        return 1;
    }

    // create project directory
    if (!fs_ensure_dir_recursive(project_id))
    {
        fprintf(stderr, "error: failed to create project directory\n");
        return 1;
    }

    // cd into it
    if (!fs_chdir(project_id))
    {
        fprintf(stderr, "error: failed to change to project directory\n");
        return 1;
    }

    // create src directory
    const char *src_dir = "src";
    if (!fs_ensure_dir_recursive(src_dir))
    {
        fprintf(stderr, "error: failed to create src directory\n");
        return 1;
    }

    // create dep directory
    const char *dep_dir = "dep";
    if (!fs_ensure_dir_recursive(dep_dir))
    {
        fprintf(stderr, "error: failed to create dep directory\n");
        return 1;
    }

    // create out directory
    const char *out_dir = "out";
    if (!fs_ensure_dir_recursive(out_dir))
    {
        fprintf(stderr, "error: failed to create out directory\n");
        return 1;
    }

    // create mach.toml file
    FILE *mach_toml = fopen("mach.toml", "w");
    if (!mach_toml)
    {
        fprintf(stderr, "error: failed to create mach.toml file\n");
        return 1;
    }

    // write templated mach.toml content
    fprintf(mach_toml, template_mach_toml, project_id, project_id, project_id);

    fclose(mach_toml);

    // cd into src directory
    if (!fs_chdir(src_dir))
    {
        fprintf(stderr, "error: failed to change to src directory\n");
        return 1;
    }

    // create main.mach file
    FILE *main_file = fopen("main.mach", "w");
    if (!main_file)
    {
        fprintf(stderr, "error: failed to create main file\n");
        return 1;
    }

    // write templated main.mach content
    fputs(template_main_mach, main_file);

    fclose(main_file);

    // TODO: initialize git repository
    // TODO: create .gitignore file
    // TODO: trigger `mach dep pull mach-std` mechanism to initialize dep directory

    printf("created a new project at '%s'\n", project_id);

    return 0;
}
