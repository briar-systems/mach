#include "commands/cmd_init.h"
#include "filesystem.h"
#include "git.h"

#include <stdio.h>
#include <string.h>

// note: update this to match standard libary convention if and when it changes
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

// note: update this to match config convention if and when it changes
static const char *template_mach_toml = "[project]\n"
                                 "id      = \"%s\"\n"
                                 "name    = \"%s\"\n"
                                 "version = \"0.1.0\"\n"
                                 "dir_src = \"src\"\n"
                                 "dir_out = \"out\"\n"
                                 "dir_dep = \"dep\"\n"
                                 "target  = \"native\"\n"
                                 "\n"
                                 "[targets.linux]\n"
                                 "os         = \"linux\"\n"
                                 "isa        = \"x86_64\"\n"
                                 "abi        = \"sysv64\"\n"
                                 "mode       = \"executable\"\n"
                                 "entrypoint = \"main.mach\"\n"
                                 "artifacts  = \"linux\"\n"
                                 "binary     = \"linux/bin/%s\"\n"
                                 "\n"
                                 "[deps.mach-std]\n"
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

    // ensure project does not already exist
    if (file_exists(project_id))
    {
        fprintf(stderr, "error: directory '%s' already exists\n", project_id);
        return 1;
    }

    // create project directory
    if (!ensure_dir_recursive(project_id))
    {
        fprintf(stderr, "error: failed to create project directory\n");
        return 1;
    }

    // cd into it
    if (!chdir_path(project_id))
    {
        fprintf(stderr, "error: failed to change to project directory\n");
        return 1;
    }

    // create src directory
    const char *src_dir = "src";
    if (!ensure_dir_recursive(src_dir))
    {
        fprintf(stderr, "error: failed to create src directory\n");
        return 1;
    }

    // create dep directory
    const char *dep_dir = "dep";
    if (!ensure_dir_recursive(dep_dir))
    {
        fprintf(stderr, "error: failed to create dep directory\n");
        return 1;
    }

    // create out directory
    const char *out_dir = "out";
    if (!ensure_dir_recursive(out_dir))
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
    if (!chdir_path(src_dir))
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

    // go back to project root
    if (!chdir_path(".."))
    {
        fprintf(stderr, "warning: failed to return to project root\n");
    }

    // initialize git repository
    printf("initializing git repository...\n");
    if (git_exec("git init") != 0)
    {
        fprintf(stderr, "warning: failed to initialize git repository\n");
    }

    // create .gitignore file
    FILE *gitignore = fopen(".gitignore", "w");
    if (gitignore)
    {
        fprintf(gitignore, "out/\n");
        fprintf(gitignore, ".DS_Store\n");
        fprintf(gitignore, "Thumbs.db\n");
        fclose(gitignore);
    }
    else
    {
        fprintf(stderr, "warning: failed to create .gitignore file\n");
    }

    // initialize mach-std dependency
    printf("initializing mach-std dependency...\n");
    const char *mach_std_path = "dep/mach-std";
    const char *mach_std_url = "https://github.com/octalide/mach-std";
    
    if (!git_submodule_init(mach_std_path, mach_std_url))
    {
        fprintf(stderr, "warning: failed to initialize mach-std submodule\n");
    }
    else
    {
        // checkout branch/main
        if (!git_checkout_version(mach_std_path, "branch/main"))
        {
            fprintf(stderr, "warning: failed to checkout mach-std version\n");
        }
    }

    printf("created a new project at '%s'\n", project_id);

    return 0;
}
