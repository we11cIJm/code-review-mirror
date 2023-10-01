#include <libgit2.h>
#include <fstream>
#include <map>

#define FILENAME = db.txt

void Add() {
    git_index *index = nullptr;
    git_repository_index(&index, repo);

    git_index_add_bypath(index, "path/to/file.txt");
    git_index_write(index);
    git_index_free(index);

}

void Commit() {
    git_signature* author;
    git_signature_create(&author, "Your name", "your.email@edu.misis.ru", time(nullptr), 0);

    git_signature* committer = author; // author == committer

    git_oid tree_id;
    git_tree* tree;
    git_index_write_tree(&tree_id, index);
    git_tree_lookup(&tree, repo, &tree_id);

    const char* message = "comment for commit";
    git_commit* parent_commit = nullptr;
    git_commit_lookup(&parent_commit, repo, "SHA of parent-commit");

    git_commit_create_v(
        nullptr, // id of created commit
        repo,
        "HEAD", // name of the branch, where commit would be added
        author,
        committer,
        nullptr, // id of parent-commit (nullptr for the 1st one)
        message,
        tree,
        parent_commit != nullptr ? 1 : 0, // count of parrent-commit
        parent_commit != nullptr ? &parent_commit : nullptr,
        &new_commit // ??? new commit
    );

    git_commit_free(parent_commit);
    git_tree_free(tree);
    git_signature_free(author);

}

void Push() {
    git_remote* remote;
    git_remote_lookup(&remote, repo, "name of remote server");

    git_push* push;
    git_push_new(&push, remote);

    git_push_add_refspec(push, "refs/heads/your-branch:refs/heads/your-branch");

    git_push_finish(push);

    git_push_free(push);
    git_remote_free(remote);

}

void Pull() {
    git_remote* remote;
    git_remote_lookup(&remote, repo, "name of remote server");

    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    git_remote_fetch(remote, nullptr, &fetch_opts, "name of the branch on the remote server");

    git_remote_free(remote);

}

void Clone() {
    // ???????
}
/*
int main() {
    std::string filename = FILENAME;
    std::fstream s{filename | s.in}; // open file for link reading 
    git_libgit2_init();
    git_repository* repo = nullptr;
    std::map<std::string, std::string> url_list;
    std::string url, path;
    while (!filename.eof()) {
        std::cin >> url >> path;
        url_list[url] = path;
    }
    for (const auto& pair : url_list) {
        git_clone(&repo, pair.first, pair.second, nullptr);
    }
    url_list.clear();
    git_libgit2_shutdown();

return 0;
}*/
