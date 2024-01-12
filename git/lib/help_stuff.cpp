#include <BlindCodeReview/git.hpp>

namespace help_stuff {
    std::pair<const git_oid*, const git_oid*> GetLastCommits(git_repository* repo) {
        git_oid local_commit_oid;
        git_reference* local_head_ref = nullptr;
        int error = git_repository_head(&local_head_ref, repo);
        if (error != 0) {
            git::CleanUp(repo);
            git_reference_free(local_head_ref);
// TODO: implemet throw ...;
//            return git::Error(git_error_last(), error, local_path_to_repo.c_str()); // no need in local_path_to_repo
        }
        const git_oid* local_head_oid = git_reference_target(local_head_ref);
        git_oid_cpy(&local_commit_oid, local_head_oid);
        git_reference_free(local_head_ref);

        git_oid remote_commit_oid;
        git_reference* remote_head_ref = nullptr;
        error = git_reference_lookup(&remote_head_ref, repo, "refs/remotes/origin/main");
        if (error != 0) {
            git::CleanUp(repo);
            git_reference_free(remote_head_ref);
// TODO: implemet throw ...;
//            return git::Error(git_error_last(), error, local_path_to_repo.c_str()); // no need in local_path_to_repo
        }
        const git_oid* remote_head_oid = git_reference_target(remote_head_ref);
        git_oid_cpy(&remote_commit_oid, remote_head_oid);
        git_reference_free(remote_head_ref);

        return std::make_pair(&local_commit_oid, &remote_commit_oid);
    }

    void MergeOptionsInit(struct MergeOptions *opts) {
        std::memset(opts, 0, sizeof(*opts));

        opts->heads = nullptr;
        opts->heads_count = 0;
        opts->annotated = nullptr;
        opts->annotated_count = 0;
    }

    void *Xrealloc(void* oldp, size_t newsz)
    {
        void* p = realloc(oldp, newsz);
        if (p == nullptr) {
            std::cerr << "Cannot allocate memory, exiting.\n";
            exit(1);
        }
        return p;
    }

    void OptsAddRefish(struct MergeOptions* opts, const char* refish)
    {
        size_t sz;

        assert(opts != nullptr);

        sz = ++opts->heads_count * sizeof(opts->heads[0]);
        opts->heads = static_cast<const char**>(Xrealloc((void*)opts->heads, sz));
        opts->heads[opts->heads_count - 1] = refish;
    }

    int ResolveRefish(git_annotated_commit** commit, git_repository* repo, const char* refish) {
        git_reference* ref;
        git_object* obj;
        int err = 0;

        assert(commit != nullptr);

        err = git_reference_dwim(&ref, repo, refish);
        if (err == GIT_OK) {
            git_annotated_commit_from_ref(commit, repo, ref);
            git_reference_free(ref);
            return 0;
        }

        err = git_revparse_single(&obj, repo, refish);
        if (err == GIT_OK) {
            err = git_annotated_commit_lookup(commit, repo, git_object_id(obj));
            git_object_free(obj);
        }

        return err;
    }

    int ResolveHeads(git_repository* repo, struct MergeOptions* opts) {
        auto annotated = static_cast<git_annotated_commit**>(calloc(opts->heads_count, sizeof(git_annotated_commit*)));
        size_t annotated_count = 0, i;
        int err = 0;

        for (i = 0; i < opts->heads_count; i++) {
            err = ResolveRefish(&annotated[annotated_count++], repo, opts->heads[i]);
            if (err != 0) {
                std::cerr << "failed to resolve refish " << opts->heads[i] << ": " << git_error_last()->message;
                annotated_count--;
                continue;
            }
        }

        if (annotated_count != opts->heads_count) {
            free(annotated);
            return -1;
        }

        opts->annotated = annotated;
        opts->annotated_count = annotated_count;
        return 0;
    }

    int UpdateCallback(const char* refname, const git_oid* a, const git_oid* b, void* data) {
        char a_str[GIT_OID_SHA1_HEXSIZE+1], b_str[GIT_OID_SHA1_HEXSIZE+1];
        (void)data;

        git_oid_fmt(b_str, b);
        b_str[GIT_OID_SHA1_HEXSIZE] = '\0';

        if (git_oid_is_zero(a)) {
            printf("[new]     %.20s %s\n", b_str, refname);
        } else {
            git_oid_fmt(a_str, a);
            a_str[GIT_OID_SHA1_HEXSIZE] = '\0';
            printf("[updated] %.10s..%.10s %s\n", a_str, b_str, refname);
        }

        return 0;
    }

    int PerformFastforward(git_repository* repo, const git_oid* target_oid, int is_unborn) {
        git_checkout_options ff_checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
        git_reference* target_ref;
        git_reference* new_target_ref;
        git_object* target = nullptr;
        int err = 0;

        if (is_unborn) {
            const char* symbolic_ref;
            git_reference* head_ref;

            /* HEAD reference is unborn, lookup manually, so we don't try to resolve it */
            err = git_reference_lookup(&head_ref, repo, "HEAD");
            if (err != 0) {
                std::cerr << "failed to lookup HEAD ref\n";
                return -1;
            }

            /* Grab the reference HEAD should be pointing to */
            symbolic_ref = git_reference_symbolic_target(head_ref);

            /* Create our master reference on the target OID */
            err = git_reference_create(&target_ref, repo, symbolic_ref, target_oid, 0, nullptr);
            if (err != 0) {
                std::cerr << "failed to create master reference\n";
                return -1;
            }

            git_reference_free(head_ref);
        } else {
            /* HEAD exists, just lookup and resolve */
            err = git_repository_head(&target_ref, repo);
            if (err != 0) {
                std::cerr << "failed to get HEAD reference\n";
                return -1;
            }
        }

        /* Lookup the target object */
        err = git_object_lookup(&target, repo, target_oid, GIT_OBJECT_COMMIT);
        if (err != 0) {
            std::cerr << "failed to lookup OID " << git_oid_tostr_s(target_oid) << std::endl;
            return -1;
        }

        /* Checkout the result so the workdir is in the expected state */
        ff_checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE;
        err = git_checkout_tree(repo, target, &ff_checkout_options);
        if (err != 0) {
            std::cerr << "failed to checkout HEAD reference\n";
            return -1;
        }

        /* Move the target reference to the target OID */
        err = git_reference_set_target(&new_target_ref, target_ref, target_oid, nullptr);
        if (err != 0) {
            std::cerr << "failed to move HEAD reference\n";
            return -1;
        }

        git_reference_free(target_ref);
        git_reference_free(new_target_ref);
        git_object_free(target);

        return 0;
    }

    int CreateMergeCommit(git_repository* repo, git_index* index, struct MergeOptions* opts) {
        git_oid tree_oid, commit_oid;
        git_tree* tree = nullptr;
        git_signature* sign = nullptr;
        git_reference* merge_ref = nullptr;
        git_annotated_commit* merge_commit = nullptr;
        git_reference* head_ref;
        auto parents = static_cast<git_commit**>(calloc(opts->annotated_count + 1, sizeof(git_commit*)));
        const char* msg_target = nullptr;
        size_t msglen = 0;
        char* msg;
        size_t i;
        int err;

        /* Grab our needed references */
        err = git_repository_head(&head_ref, repo);
        if (err != 0) {
            std::cout << "failed to get repo HEAD" << std::endl;
            git::CleanUp( repo, index, tree, nullptr
                    , head_ref, merge_ref, sign
                    , nullptr, nullptr, nullptr
                    , nullptr, nullptr, merge_commit);
            free(parents);
            return err;
        }
        if (ResolveRefish(&merge_commit, repo, opts->heads[0])) {
            std::cerr << "failed to resolve refish " << opts->heads[0];
            free(parents);
            return -1;
        }

        /* Maybe that's a ref, so DWIM it */
        err = git_reference_dwim(&merge_ref, repo, opts->heads[0]);
        if (err != 0) {
            std::cout << "failed to DWIM reference" << std::endl;
            git::CleanUp( repo, index, tree, nullptr
                    , head_ref, merge_ref, sign
                    , nullptr, nullptr, nullptr
                    , nullptr, nullptr, merge_commit);
            free(parents);
            return err;
        }

        /* Grab a signature */
        err = git_signature_default(&sign, repo);
        if (err != 0) {
            std::cout << "failed to create signature" << std::endl;
            git::CleanUp( repo, index, tree, nullptr
                    , head_ref, merge_ref, sign
                    , nullptr, nullptr, nullptr
                    , nullptr, nullptr, merge_commit);
            free(parents);
            return err;
        }

#define MERGE_COMMIT_MSG "Merge %s '%s'"
        /* Prepare a standard merge commit message */
        if (merge_ref != nullptr) {
            err = git_branch_name(&msg_target, merge_ref);
            if (err != 0) {
                std::cout << "failed to get branch name of merged ref" << std::endl;
                git::CleanUp( repo, index, tree, nullptr
                        , head_ref, merge_ref, sign
                        , nullptr, nullptr, nullptr
                        , nullptr, nullptr, merge_commit);
                free(parents);
                return err;
            }
        } else {
            msg_target = git_oid_tostr_s(git_annotated_commit_id(merge_commit));
        }

        msglen = snprintf(nullptr, 0, MERGE_COMMIT_MSG, (merge_ref ? "branch" : "commit"), msg_target);
        if (msglen > 0) msglen++;
        msg = static_cast<char*>(malloc(msglen));
        err = snprintf(msg, msglen, MERGE_COMMIT_MSG, (merge_ref ? "branch" : "commit"), msg_target);

        /* This is only to silence the compiler */
        if (err < 0) {
            free(parents);
            return err;
        }

        /* Setup our parent commits */
        err = git_reference_peel((git_object**)&parents[0], head_ref, GIT_OBJECT_COMMIT);
        if (err != 0) {
            std::cout << "failed to peel head reference" << std::endl;
            git::CleanUp( repo, index, tree, nullptr
                    , head_ref, merge_ref, sign
                    , nullptr, nullptr, nullptr
                    , nullptr, nullptr, merge_commit);
            free(parents);
            return err;
        }
        for (i = 0; i < opts->annotated_count; i++) {
            git_commit_lookup(&parents[i + 1], repo, git_annotated_commit_id(opts->annotated[i]));
        }

        /* Prepare our commit tree */
        err = git_index_write_tree(&tree_oid, index);
        if (err != 0) {
            std::cout << "failed to write merged tree" << std::endl;
            git::CleanUp( repo, index, tree, nullptr
                    , head_ref, merge_ref, sign
                    , nullptr, nullptr, nullptr
                    , nullptr, nullptr, merge_commit);
            free(parents);
            return err;
        }

        err = git_tree_lookup(&tree, repo, &tree_oid);
        if (err != 0) {
            std::cout << "failed to lookup tree" << std::endl;
            git::CleanUp( repo, index, tree, nullptr
                    , head_ref, merge_ref, sign
                    , nullptr, nullptr, nullptr
                    , nullptr, nullptr, merge_commit);
            free(parents);
            return err;
        }

        /* Commit time ! */
        err = git_commit_create(&commit_oid,
                                repo, git_reference_name(head_ref),
                                sign, sign,
                                nullptr, msg,
                                tree,
                                opts->annotated_count + 1, (const git_commit **)parents);
        if (err != 0) {
            std::cout << "failed to create commit" << std::endl;
            git::CleanUp( repo, index, tree, nullptr
                    , head_ref, merge_ref, sign
                    , nullptr, nullptr, nullptr
                    , nullptr, nullptr, merge_commit);
            free(parents);
            return err;
        }

        /* We're done merging, cleanup the repository state */
        git_repository_state_cleanup(repo);

        return 0;
    }
} // namespace help_stuff