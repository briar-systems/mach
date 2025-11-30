#include "config.h"
#include "filesystem.h"
#include "toml.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// helper: find target mode by string value
static ConfigTargetMode *find_target_mode(const char *value)
{
    if (!value)
    {
        return NULL;
    }
    for (size_t i = 0; i < sizeof(TARGET_MODES) / sizeof(TARGET_MODES[0]); i++)
    {
        if (strcmp(TARGET_MODES[i].value, value) == 0)
        {
            return (ConfigTargetMode *)&TARGET_MODES[i];
        }
    }
    return NULL;
}

// helper: find target os by string value
static MIRTargetOS find_target_os(const char *value)
{
    if (!value)
    {
        return MIR_OS_COUNT;
    }
    
    MIRTargetOS os = mir_target_os_from_name(value);
    return os;
}

// helper: find target isa by string value
static MIRTargetISA find_target_isa(const char *value)
{
    if (!value)
    {
        return MIR_ISA_COUNT;
    }
    
    MIRTargetISA isa = mir_target_isa_from_name(value);
    return isa;
}

// helper: find target abi by string value
static MIRTargetABI find_target_abi(const char *value)
{
    if (!value)
    {
        return MIR_ABI_COUNT;
    }
    
    MIRTargetABI abi = mir_target_abi_from_name(value);
    return abi;
}

// helper: find dependency type by string value
static ConfigDepType *find_dep_type(const char *value)
{
    if (!value)
    {
        return NULL;
    }
    for (size_t i = 0; i < sizeof(DEP_TYPES) / sizeof(DEP_TYPES[0]); i++)
    {
        if (strcmp(DEP_TYPES[i].value, value) == 0)
        {
            return (ConfigDepType *)&DEP_TYPES[i];
        }
    }
    return NULL;
}

// helper: determine dependency version kind
// formats are:
// `branch/<name>` for branches
// `<semver>` for semantic versions (includes comparison operators)
// `commit/<40-char-hex>` for commit hashes
static ConfigDepVersionKind determine_version_kind(const char *version)
{
    if (!version)
    {
        return DEP_VERSION_KIND_SEMVER;
    }

    if (strncmp(version, "branch/", 7) == 0)
    {
        return DEP_VERSION_KIND_BRANCH;
    }
    else if (strncmp(version, "commit/", 7) == 0)
    {
        const char *hash = version + 7;
        size_t      len  = strlen(hash);
        if (len == 40)
        {
            // check if all characters are hex digits
            for (size_t i = 0; i < len; i++)
            {
                if (!((hash[i] >= '0' && hash[i] <= '9') || (hash[i] >= 'a' && hash[i] <= 'f') || (hash[i] >= 'A' && hash[i] <= 'F')))
                {
                    return DEP_VERSION_KIND_SEMVER;
                }
            }
            return DEP_VERSION_KIND_COMMIT;
        }
    }

    return DEP_VERSION_KIND_SEMVER;
}

// target management
void target_config_init(ConfigTarget *target)
{
    target->name       = NULL;
    target->os   = MIR_OS_COUNT;
    target->isa       = MIR_ISA_COUNT;
    target->abi       = MIR_ABI_COUNT;
    target->mode       = NULL;
    target->entrypoint = NULL;
    target->artifacts  = NULL;
    target->binary     = NULL;
}

void target_config_dnit(ConfigTarget *target)
{
    if (!target)
    {
        return;
    }
    free(target->name);
    free(target->entrypoint);
    free(target->artifacts);
    free(target->binary);
}

// dependency management
void dep_spec_init(ConfigDep *dep)
{
    dep->name    = NULL;
    dep->type    = NULL;
    dep->path    = NULL;
    dep->version = NULL;
    dep->config  = NULL;
}

void dep_spec_dnit(ConfigDep *dep)
{
    if (!dep)
    {
        return;
    }
    free(dep->name);
    free(dep->path);
    if (dep->version)
    {
        free((void *)dep->version->value);
        free(dep->version);
    }
    if (dep->config)
    {
        config_dnit(dep->config);
        free(dep->config);
    }
}

// configuration lifecycle
void config_init(Config *config)
{
    config->id           = NULL;
    config->name         = NULL;
    config->version      = NULL;
    config->dir_src      = NULL;
    config->dir_out      = NULL;
    config->dir_dep      = NULL;
    config->target       = NULL;
    config->targets      = NULL;
    config->target_count = 0;
    config->deps         = NULL;
    config->dep_count    = 0;
}

void config_dnit(Config *config)
{
    if (!config)
    {
        return;
    }
    free(config->id);
    free(config->name);
    free(config->version);
    free(config->dir_src);
    free(config->dir_out);
    free(config->dir_dep);
    free(config->target);

    for (int i = 0; i < config->target_count; i++)
    {
        target_config_dnit(config->targets[i]);
        free(config->targets[i]);
    }
    free(config->targets);

    for (int i = 0; i < config->dep_count; i++)
    {
        dep_spec_dnit(config->deps[i]);
        free(config->deps[i]);
    }
    free(config->deps);
}

// configuration file management
// internal helper for recursive loading
static Config *config_load_internal(const char *config_path, const char *project_root, bool load_deps);

Config *config_load(const char *config_path)
{
    // get project root from config path
    char *abs_config_path = absolutize_path(config_path);
    if (!abs_config_path)
    {
        return NULL;
    }
    
    char *project_root = strdup(abs_config_path);
    char *last_sep = strrchr(project_root, '/');
    if (last_sep)
    {
        *last_sep = '\0';
    }
    
    Config *config = config_load_internal(abs_config_path, project_root, true);
    
    free(abs_config_path);
    free(project_root);
    
    return config;
}

static Config *config_load_internal(const char *config_path, const char *project_root, bool load_deps)
{
    char *content = read_file(config_path);
    if (!content)
    {
        fprintf(stderr, "error: failed to read config file: %s\n", config_path);
        return NULL;
    }

    char         *error = NULL;
    toml_table_t *root  = toml_parse(content, &error);
    free(content);

    if (!root)
    {
        fprintf(stderr, "error: failed to parse config: %s\n", error ? error : "unknown");
        free(error);
        return NULL;
    }

    Config *config = malloc(sizeof(Config));
    config_init(config);

    // parse project section
    toml_value_t *project_val = toml_table_get(root, "project");
    toml_table_t *project = project_val && toml_value_is_table(project_val) ? project_val->as.table : root;

    toml_value_t *id = toml_table_get(project, "id");
    if (id && toml_value_is_string(id))
    {
        config->id = strdup(id->as.string);
    }

    toml_value_t *name = toml_table_get(project, "name");
    if (name && toml_value_is_string(name))
    {
        config->name = strdup(name->as.string);
    }

    toml_value_t *version = toml_table_get(project, "version");
    if (version && toml_value_is_string(version))
    {
        config->version = strdup(version->as.string);
    }

    toml_value_t *dir_src = toml_table_get(project, "dir_src");
    if (dir_src && toml_value_is_string(dir_src))
    {
        config->dir_src = strdup(dir_src->as.string);
    }

    toml_value_t *dir_out = toml_table_get(project, "dir_out");
    if (dir_out && toml_value_is_string(dir_out))
    {
        config->dir_out = strdup(dir_out->as.string);
    }

    toml_value_t *dir_dep = toml_table_get(project, "dir_dep");
    if (dir_dep && toml_value_is_string(dir_dep))
    {
        config->dir_dep = strdup(dir_dep->as.string);
    }

    toml_value_t *target = toml_table_get(project, "target");
    if (target && toml_value_is_string(target))
    {
        config->target = strdup(target->as.string);
    }

    // parse targets table
    toml_value_t *targets_val = toml_table_get(root, "targets");
    if (targets_val && toml_value_is_table(targets_val))
    {
        toml_table_t *targets_table = targets_val->as.table;
        config->target_count        = targets_table->count;
        config->targets             = malloc(sizeof(ConfigTarget *) * config->target_count);

        for (int i = 0; i < config->target_count; i++)
        {
            config->targets[i] = malloc(sizeof(ConfigTarget));
            target_config_init(config->targets[i]);

            toml_entry_t *entry      = &targets_table->entries[i];
            config->targets[i]->name = strdup(entry->key);

            if (toml_value_is_table(&entry->value))
            {
                toml_table_t *target_table = entry->value.as.table;

                toml_value_t *os = toml_table_get(target_table, "os");
                if (os && toml_value_is_string(os))
                {
                    config->targets[i]->os = find_target_os(os->as.string);
                }

                toml_value_t *isa = toml_table_get(target_table, "isa");
                if (isa && toml_value_is_string(isa))
                {
                    config->targets[i]->isa = find_target_isa(isa->as.string);
                }

                toml_value_t *abi = toml_table_get(target_table, "abi");
                if (abi && toml_value_is_string(abi))
                {
                    config->targets[i]->abi = find_target_abi(abi->as.string);
                }

                toml_value_t *mode = toml_table_get(target_table, "mode");
                if (mode && toml_value_is_string(mode))
                {
                    config->targets[i]->mode = find_target_mode(mode->as.string);
                }

                toml_value_t *entrypoint = toml_table_get(target_table, "entrypoint");
                if (entrypoint && toml_value_is_string(entrypoint))
                {
                    config->targets[i]->entrypoint = strdup(entrypoint->as.string);
                }

                toml_value_t *artifacts = toml_table_get(target_table, "artifacts");
                if (artifacts && toml_value_is_string(artifacts))
                {
                    config->targets[i]->artifacts = strdup(artifacts->as.string);
                }

                toml_value_t *binary = toml_table_get(target_table, "binary");
                if (binary && toml_value_is_string(binary))
                {
                    config->targets[i]->binary = strdup(binary->as.string);
                }
            }
        }
    }

    // parse deps table
    toml_value_t *deps_val = toml_table_get(root, "deps");
    if (deps_val && toml_value_is_table(deps_val))
    {
        toml_table_t *deps_table = deps_val->as.table;
        config->dep_count        = deps_table->count;
        config->deps             = malloc(sizeof(ConfigDep *) * config->dep_count);

        for (int i = 0; i < config->dep_count; i++)
        {
            config->deps[i] = malloc(sizeof(ConfigDep));
            dep_spec_init(config->deps[i]);

            toml_entry_t *entry   = &deps_table->entries[i];
            config->deps[i]->name = strdup(entry->key);

            if (toml_value_is_table(&entry->value))
            {
                toml_table_t *dep_table = entry->value.as.table;

                toml_value_t *type = toml_table_get(dep_table, "type");
                if (type && toml_value_is_string(type))
                {
                    config->deps[i]->type = find_dep_type(type->as.string);
                }

                toml_value_t *path = toml_table_get(dep_table, "path");
                if (!path)
                {
                    path = toml_table_get(dep_table, "source");
                }
                if (path && toml_value_is_string(path))
                {
                    config->deps[i]->path = strdup(path->as.string);
                }

                toml_value_t *version = toml_table_get(dep_table, "version");
                if (version && toml_value_is_string(version))
                {
                    ConfigDepVersion *dep_version = malloc(sizeof(ConfigDepVersion));
                    dep_version->kind             = determine_version_kind(version->as.string);
                    dep_version->value            = strdup(version->as.string);
                    config->deps[i]->version      = dep_version;
                }
            }
        }
    }

    // load dependency configs if requested
    if (load_deps && config->dep_count > 0 && config->dir_dep && project_root)
    {
        for (int i = 0; i < config->dep_count; i++)
        {
            ConfigDep *dep = config->deps[i];
            if (dep && dep->name)
            {
                // construct path to dependency: project_root/dir_dep/dep_name/mach.toml
                char dep_config_path[1024];
                snprintf(dep_config_path, sizeof(dep_config_path), "%s/%s/%s/mach.toml", 
                         project_root, config->dir_dep, dep->name);
                
                // construct dependency project root for recursive loading
                char dep_project_root[1024];
                snprintf(dep_project_root, sizeof(dep_project_root), "%s/%s/%s", 
                         project_root, config->dir_dep, dep->name);
                
                // load dependency config (without loading its dependencies to avoid deep recursion)
                dep->config = config_load_internal(dep_config_path, dep_project_root, false);
                // if loading fails, continue (dependency might not be fetched yet)
            }
        }
    }

    toml_table_free(root);

    // validate mandatory fields in [project]
    if (!config->id)
    {
        fprintf(stderr, "error: missing mandatory field 'id' in [project]\n");
        config_dnit(config);
        free(config);
        return NULL;
    }
    if (!config->name)
    {
        fprintf(stderr, "error: missing mandatory field 'name' in [project]\n");
        config_dnit(config);
        free(config);
        return NULL;
    }
    if (!config->version)
    {
        fprintf(stderr, "error: missing mandatory field 'version' in [project]\n");
        config_dnit(config);
        free(config);
        return NULL;
    }
    if (!config->dir_src)
    {
        fprintf(stderr, "error: missing mandatory field 'dir_src' in [project]\n");
        config_dnit(config);
        free(config);
        return NULL;
    }
    if (!config->dir_out)
    {
        fprintf(stderr, "error: missing mandatory field 'dir_out' in [project]\n");
        config_dnit(config);
        free(config);
        return NULL;
    }
    if (!config->dir_dep)
    {
        fprintf(stderr, "error: missing mandatory field 'dir_dep' in [project]\n");
        config_dnit(config);
        free(config);
        return NULL;
    }
    if (!config->target)
    {
        fprintf(stderr, "error: missing mandatory field 'target' in [project]\n");
        config_dnit(config);
        free(config);
        return NULL;
    }

    // validate mandatory fields in targets
    for (int i = 0; i < config->target_count; i++)
    {
        ConfigTarget *t = config->targets[i];
        if (!t->name)
        {
            fprintf(stderr, "error: target %d missing name\n", i);
            config_dnit(config);
            free(config);
            return NULL;
        }
        if (t->os == MIR_OS_COUNT)
        {
            fprintf(stderr, "error: target '%s' missing mandatory field 'os'\n", t->name);
            config_dnit(config);
            free(config);
            return NULL;
        }
        if (t->isa == MIR_ISA_COUNT)
        {
            fprintf(stderr, "error: target '%s' missing mandatory field 'isa'\n", t->name);
            config_dnit(config);
            free(config);
            return NULL;
        }
        if (t->abi == MIR_ABI_COUNT)
        {
            fprintf(stderr, "error: target '%s' missing mandatory field 'abi'\n", t->name);
            config_dnit(config);
            free(config);
            return NULL;
        }
        if (!t->mode)
        {
            fprintf(stderr, "error: target '%s' missing mandatory field 'mode'\n", t->name);
            config_dnit(config);
            free(config);
            return NULL;
        }
        if (!t->entrypoint)
        {
            fprintf(stderr, "error: target '%s' missing mandatory field 'entrypoint'\n", t->name);
            config_dnit(config);
            free(config);
            return NULL;
        }
        if (!t->binary)
        {
            fprintf(stderr, "error: target '%s' missing mandatory field 'binary'\n", t->name);
            config_dnit(config);
            free(config);
            return NULL;
        }
    }

    // validate mandatory fields in deps
    for (int i = 0; i < config->dep_count; i++)
    {
        ConfigDep *d = config->deps[i];
        if (!d->name)
        {
            fprintf(stderr, "error: dependency %d missing name\n", i);
            config_dnit(config);
            free(config);
            return NULL;
        }
        if (!d->type)
        {
            fprintf(stderr, "error: dependency '%s' missing mandatory field 'type'\n", d->name);
            config_dnit(config);
            free(config);
            return NULL;
        }
        if (!d->path)
        {
            fprintf(stderr, "error: dependency '%s' missing mandatory field 'path'\n", d->name);
            config_dnit(config);
            free(config);
            return NULL;
        }
        // version is mandatory for remote dependencies
        if (d->type->kind == DEP_TYPE_REMOTE && !d->version)
        {
            fprintf(stderr, "error: remote dependency '%s' missing mandatory field 'version'\n", d->name);
            config_dnit(config);
            free(config);
            return NULL;
        }
    }

    return config;
}

bool config_save(Config *config, const char *config_path)
{
    if (!config)
    {
        return false;
    }

    FILE *f = fopen(config_path, "w");
    if (!f)
    {
        return false;
    }

    // write project section
    fprintf(f, "[project]\n");
    if (config->id)
    {
        fprintf(f, "id = \"%s\"\n", config->id);
    }
    if (config->name)
    {
        fprintf(f, "name = \"%s\"\n", config->name);
    }
    if (config->version)
    {
        fprintf(f, "version = \"%s\"\n", config->version);
    }
    if (config->dir_src)
    {
        fprintf(f, "dir_src = \"%s\"\n", config->dir_src);
    }
    if (config->dir_dep)
    {
        fprintf(f, "dir_dep = \"%s\"\n", config->dir_dep);
    }
    if (config->dir_out)
    {
        fprintf(f, "dir_out = \"%s\"\n", config->dir_out);
    }
    if (config->target)
    {
        fprintf(f, "target = \"%s\"\n", config->target);
    }

    // write targets
    for (int i = 0; i < config->target_count; i++)
    {
        ConfigTarget *target = config->targets[i];
        if (target->name)
        {
            fprintf(f, "\n[targets.%s]\n", target->name);
            if (target->os != MIR_OS_COUNT)
            {
                fprintf(f, "os = \"%s\"\n", mir_target_os_name(target->os));
            }
            if (target->isa != MIR_ISA_COUNT)
            {
                fprintf(f, "isa = \"%s\"\n", mir_target_isa_name(target->isa));
            }
            if (target->abi != MIR_ABI_COUNT)
            {
                fprintf(f, "abi = \"%s\"\n", mir_target_abi_name(target->abi));
            }
            if (target->mode)
            {
                fprintf(f, "mode = \"%s\"\n", target->mode->value);
            }
            if (target->entrypoint)
            {
                fprintf(f, "entrypoint = \"%s\"\n", target->entrypoint);
            }
            if (target->artifacts)
            {
                fprintf(f, "artifacts = \"%s\"\n", target->artifacts);
            }
            if (target->binary)
            {
                fprintf(f, "binary = \"%s\"\n", target->binary);
            }
        }
    }

    // write deps
    for (int i = 0; i < config->dep_count; i++)
    {
        ConfigDep *dep = config->deps[i];
        if (dep->name)
        {
            fprintf(f, "\n[deps.%s]\n", dep->name);
            if (dep->type)
            {
                fprintf(f, "type = \"%s\"\n", dep->type->value);
            }
            if (dep->path)
            {
                fprintf(f, "path = \"%s\"\n", dep->path);
            }
            if (dep->version && dep->version->value)
            {
                fprintf(f, "version = \"%s\"\n", dep->version->value);
            }
        }
    }

    fclose(f);
    return true;
}

bool config_add_target(Config *config, ConfigTarget *target)
{
    if (!config || !target || !target->name)
    {
        return false;
    }

    // check if target already exists
    for (int i = 0; i < config->target_count; i++)
    {
        if (config->targets[i]->name && strcmp(config->targets[i]->name, target->name) == 0)
        {
            return false;
        }
    }

    // add to config
    config->targets                         = realloc(config->targets, sizeof(ConfigTarget *) * (config->target_count + 1));
    config->targets[config->target_count++] = target;

    return true;
}

ConfigTarget *config_get_target(Config *config, const char *name)
{
    if (!config || !name)
    {
        return NULL;
    }

    // "native" special case
    if (strcmp(name, "native") == 0)
    {
        MIRTarget target = mir_target_native();
        
        // match target to config target based on OS, ISA, and ABI
        for (int i = 0; i < config->target_count; i++)
        {
            ConfigTarget *cfg_target = config->targets[i];
            if (cfg_target->os == target.os &&
                cfg_target->isa == target.isa &&
                cfg_target->abi == target.abi)
            {
                return cfg_target;
            }
        }

        return NULL;
    }


    for (int i = 0; i < config->target_count; i++)
    {
        if (config->targets[i]->name && strcmp(config->targets[i]->name, name) == 0)
        {
            return config->targets[i];
        }
    }

    return NULL;
}

bool config_add_dependency(Config *config, ConfigDep *dep)
{
    if (!config || !dep || !dep->name)
    {
        return false;
    }

    // check if dependency already exists
    for (int i = 0; i < config->dep_count; i++)
    {
        if (config->deps[i]->name && strcmp(config->deps[i]->name, dep->name) == 0)
        {
            return false;
        }
    }

    // add to config
    config->deps                      = realloc(config->deps, sizeof(ConfigDep *) * (config->dep_count + 1));
    config->deps[config->dep_count++] = dep;

    return true;
}

ConfigDep *config_get_dependency(Config *config, const char *name)
{
    if (!config || !name)
    {
        return NULL;
    }

    for (int i = 0; i < config->dep_count; i++)
    {
        if (config->deps[i]->name && strcmp(config->deps[i]->name, name) == 0)
        {
            return config->deps[i];
        }
    }

    return NULL;
}

bool config_del_dependency(Config *config, const char *name)
{
    if (!config || !name)
    {
        return false;
    }

    for (int i = 0; i < config->dep_count; i++)
    {
        if (config->deps[i]->name && strcmp(config->deps[i]->name, name) == 0)
        {
            // free the dependency
            dep_spec_dnit(config->deps[i]);
            free(config->deps[i]);

            // shift remaining dependencies
            for (int j = i; j < config->dep_count - 1; j++)
            {
                config->deps[j] = config->deps[j + 1];
            }

            config->dep_count--;
            config->deps = realloc(config->deps, sizeof(ConfigDep *) * config->dep_count);

            return true;
        }
    }

    return false;
}
