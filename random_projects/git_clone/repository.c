#include "repository.h"
#include "utils.h"

// force=false
GitRepository *new_git_repository(const char *path, bool force){
    GitRepository *repo = malloc(sizeof(GitRepository));
    if (repo == NULL){
        fprintf(stderr, "unable to allocate memory for reposiory struct in new_git_repository\n");
        return NULL;
    }

    char *worktree = malloc(sizeof(char) * (strlen(path) + 1));
    if (worktree == NULL){
        fprintf(stderr, "unable to allocate memory for worktree path in new_git_repository\n");
        free(repo);
        return NULL;
    }

    strcpy(worktree, path);
    char *gitdir = join_path(worktree, 1, ".git");
    if (gitdir == NULL){
        fprintf(stderr, "unable to create gitdir from worktree in new_git_repository\n");
        free(repo);
        free(worktree);
        return NULL;
    }

    repo->worktree = worktree;
    repo->gitdir = gitdir;

    if (!(force || is_dir(gitdir))){
        fprintf(stderr, "path is not git repository %s\n", path);
        free(repo);
        free(worktree);
        free(gitdir);
        return NULL;
    }

    char *cf = repo_file(repo, false, 1, "config");
    if (PATH_EXISTS(cf)){
        repo->config = ini_parse(cf);
        if (repo->config == NULL){
            fprintf(stderr, "could not parse configuration file\n");
            free(repo);
            free(worktree);
            free(gitdir);
            if (cf != NULL){
                free(cf);
            }
            return NULL;
        }
    }else if (!force){
        fprintf(stderr, "configuration file is missing\n");
        free(repo);
        free(worktree);
        free(gitdir);
        if (cf != NULL){
            free(cf);
        }
        return NULL;
    }else{
        repo->config = NULL;
    }
    if (cf != NULL){
        free(cf);
    }

    if (!force){
        Ht_item *ret = ini_get_key(repo->config, "core", "repositoryformatversion");
        if (ret == NULL){
            fprintf(stderr, "could not find repositoryformationversion\n");
            free(repo);
            free(worktree);
            free(gitdir);
            if (cf != NULL){
                free(cf);
            }
            return NULL;
        }
        /*
        if (ret->value_type != TYPE_INT){
            fprintf(stderr, "repositoryformationversion value must be int %d\n", ret->value_type);
            free(repo);
            free(worktree);
            free(gitdir);
            if (cf != NULL){
                free(cf);
            }
            return NULL;
        }
        */

        char *vers_str = (char *)ret->value;
        int vers = atoi(vers_str);
        if (vers != 0){
            fprintf(stderr, "unsupported repositoryformationversion: %d\n", vers);
            free(repo);
            free(worktree);
            free(gitdir);
            if (cf != NULL){
                free(cf);
            }
            return NULL;
        }
    }

    return repo;
}

void free_repo(GitRepository *repo){
    free(repo->worktree);
    free(repo->gitdir);
    if (repo->config != NULL){
        free_ini(repo->config);
    }
    free(repo);
}

char *repo_path(GitRepository *repo, unsigned int count, ...){
    va_list va;
    va_start(va, count);
    char *res = join_path_(repo->gitdir, count, va);
    va_end(va);
    return res;
}

char *repo_path_variadic(GitRepository *repo, unsigned int count, va_list va){
    char *res = join_path_(repo->gitdir, count, va);
    return res;
}

// mkdir=false
char *repo_file(GitRepository *repo, bool mkdir, unsigned int count, ...){
    va_list va;
    va_start(va, count);
    char *res = repo_dir_variadic(repo, mkdir, count - 1, va);
    va_end(va);

    if (res != NULL){
        va_start(va, count);
        free(res);
        res = repo_path_variadic(repo, count, va);
        va_end(va);
    }

    return res;
}

// mkdir=false
char *repo_dir_variadic(GitRepository *repo, bool mkdir, unsigned int count, va_list va){
    char *path = repo_path_variadic(repo, count, va);
    if (path == NULL){
        fprintf(stderr, "path is null from repo_path_varadic in repo_dir_variadic\n");
        return NULL;
    }

    if (PATH_EXISTS(path)){
        if (is_dir(path)){
            return path;
        }else{
            fprintf(stderr, "not a direcotry %s\n", path);
            free(path);
            exit(1);
        }
    }

    if (mkdir){
        if (make_directories(path) != 1){
            fprintf(stderr, "error making dir for %s\n", path);
            free(path);
            exit(1);
        }
        return path;
    }

    free(path);
    return NULL;
}

// mkdir=false
char *repo_dir(GitRepository *repo, bool mkdir, unsigned int count, ...){
    va_list va;
    va_start(va, count);
    char *path = repo_path_variadic(repo, count, va);
    va_end(va);
    if (path == NULL){
        fprintf(stderr, "path is null from repo_path_varadic in repo_dir\n");
        return NULL;
    }

    if (PATH_EXISTS(path)){
        if (is_dir(path)){
            return path;
        }else{
            fprintf(stderr, "not a direcotry %s\n", path);
            free(path);
            exit(1);
        }
    }

    if (mkdir == true){
        if (make_directories(path) != 1){
            fprintf(stderr, "error making dir for %s\n", path);
            free(path);
            exit(1);
        }
        return path;
    }

    free(path);
    return NULL;
}

GitRepository *repo_create(char *path){
    GitRepository *repo = new_git_repository(path, true);

    if (PATH_EXISTS(repo->worktree)){
        if (!is_dir(repo->worktree)){
            fprintf(stderr, "not a directory %s\n", repo->worktree);
            free_repo(repo);
            return NULL;
        }
        if (PATH_EXISTS(repo->gitdir) &&  !is_dir_empty(repo->gitdir)){
            fprintf(stderr, "git directory not empty %s\n", repo->worktree);
            free_repo(repo);
            return NULL;
        }
    }else{
        make_directories(repo->worktree);
    }

    char *filename;
    filename = repo_dir(repo, true, 1, "branches");
    free(filename);
    filename = repo_dir(repo, true, 1, "objects");
    free(filename);
    filename = repo_dir(repo, true, 1, "refs", "tags");
    free(filename);
    filename = repo_dir(repo, true, 1, "refs", "heads");
    free(filename);

    FILE *f;
    filename = repo_file(repo, false, 1, "description");
    f = fopen(filename, "wb");
    if (f == NULL){
        fprintf(stderr, "unable to open file %s", filename);
        free_repo(repo);
        free(filename);
        return NULL;
    }

    size_t data_len = strlen("Unnamed repository; edit this file 'description' to name the repository.\n");
    fwrite("Unnamed repository; edit this file 'description' to name the repository.\n", sizeof(char), data_len, f);

    free(filename);
    fclose(f);

    filename = repo_file(repo, false, 1, "HEAD");
    f = fopen(filename, "wb");
    if (f == NULL){
        fprintf(stderr, "unable to open file %s", filename);
        free_repo(repo);
        free(filename);
        return NULL;
    }

    data_len = strlen("ref: refs/heads/master\n");
    fwrite("ref: refs/heads/master\n", sizeof(char), data_len, f);

    free(filename);
    fclose(f);

    filename = repo_file(repo, false, 1, "config");
    f = fopen(filename, "ab");
    if (f == NULL){
        fprintf(stderr, "unable to open file %s", filename);
        free_repo(repo);
        free(filename);
        return NULL;
    }

    data_len = strlen("");
    Ini *ini = ini_new(filename);
    if (ini == NULL){
        fprintf(stderr, "unable to allocate memory for ini %s", filename);
        free_repo(repo);
        free(filename);
        return NULL;
    }

    ini_add_section(ini, "core");
    ini_set_key(ini, "core", "repositoryformatversion", "0");
    ini_set_key(ini, "core", "filemode", "false");
    ini_set_key(ini, "core", "bare", "false");
    ini_write(ini, f);

    free(filename);
    fclose(f);
    free_ini(ini);

    return repo;
}

// path=".", required=true
GitRepository *repo_find(char *path, bool required){
    char *real_path = realpath(path, NULL);
    if (real_path == NULL){
        fprintf(stderr, "realpath failed for repo_find %s\n", path);
        return NULL;
    }

    char *git_dir = join_path(real_path, 1, ".git");
    if (git_dir == NULL){
        fprintf(stderr, "joinpath failed for repo_find %s real_path %s\n", path, real_path);
        free(real_path);
        return NULL;
    }

    if (is_dir(git_dir)){
        GitRepository *repo = new_git_repository(real_path, false);
        free(git_dir);
        free(real_path);

        return repo;
    }
    free(git_dir);

    char *parent = join_path(real_path, 1, "..");
    if (parent == NULL){
        fprintf(stderr, "join_path fialed for repo_find %s\n", path);
        free(real_path);
        return NULL;
    }

    free(real_path);
    // FIXME: this does not work like this
    if (strcmp(real_path, parent) == 0){
        free(parent);
        if (required){
            fprintf(stderr, "no git directory\n");
        }
        return NULL;
    }

    return repo_find(parent, required);
}
