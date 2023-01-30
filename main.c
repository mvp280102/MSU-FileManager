#include "os-file.h"

#include <stdio.h>

int main()
{
    file_manager_t file_manager;
    char *current_directory = (char*)malloc(sizeof(char) * 64);

    if (current_directory != NULL)
    {
        setup_file_manager(&file_manager);

        printf("Author test:\n\n");

        printf("create(1024) = %d\n", file_manager.create(1024));
        printf("create_dir(\"/home\") = %d\n", file_manager.create_dir("/home"));
        printf("create_dir(\"/home/dir1\") = %d\n", file_manager.create_dir("/home/dir1"));
        printf("change_dir(\"/home/dir1\") = %d\n", file_manager.change_dir("/home/dir1"));
        printf("create_dir(\"dir11\") = %d\n", file_manager.create_dir("dir11"));
        printf("create_dir(\"dir12\") = %d\n", file_manager.create_dir("dir12"));
        printf("change_dir(\"..\") = %d\n", file_manager.change_dir(".."));
        printf("create_dir(\"/home/dir2\") = %d\n\n", file_manager.create_dir("/home/dir2"));

        file_manager.get_cur_dir(current_directory);
        printf("Current directory = %s\n\n", current_directory);

        printf("change_dir(\"/home/dir2\") = %d\n\n", file_manager.change_dir("/home/dir2"));

        file_manager.get_cur_dir(current_directory);
        printf("Current directory = %s\n\n", current_directory);

        printf("create_file(\"file21\", 16) = %d\n", file_manager.create_file("file21", 16));
        printf("create_file(\"file22\", 8) = %d\n", file_manager.create_file("file22", 8));
        printf("list(\"/\", 1) = %d\n", file_manager.list("/", 1));
        printf("list(\"/home\", 1) = %d\n", file_manager.list("/home", 1));
        printf("list(\"/ /home\", 1) = %d\n", file_manager.list("/ /home", 1));
        printf("remove_dir_file(\"/home/dir2/file22\", 0) = %d\n", file_manager.remove("/home/dir2/file22", 0));
        printf("remove_dir_file(\"/home/dir2/file21\", 0) = %d\n", file_manager.remove("/home/dir2/file21", 0));
        printf("change_dir(\"/home/dir1\") = %d\n", file_manager.change_dir("/home/dir1"));
        printf("remove_dir_file(\"/home/dir1/di11\", 1) = %d\n\n", file_manager.remove("/home/dir1/dir11", 1));

        file_manager.get_cur_dir(current_directory);
        printf("Current directory = %s\n\n", current_directory);

        printf("change_dir(\"/home\") = %d\n\n", file_manager.change_dir("/home"));

        file_manager.get_cur_dir(current_directory);
        printf("Current directory = %s\n\n", current_directory);

        printf("remove_dir_file(\"/home/dir1\", 1) = %d\n", file_manager.remove("/home/dir1", 1));
        printf("remove_dir_file(\"/home/dir2\", 1) = %d\n\n", file_manager.remove("/home/dir2", 1));

        file_manager.get_cur_dir(current_directory);
        printf("Current directory = %s\n\n", current_directory);

        printf("change_dir(\"/\") = %d\n\n", file_manager.change_dir("/"));

        file_manager.get_cur_dir(current_directory);
        printf("Current directory = %s\n\n", current_directory);

        printf("remove_dir_file(\"/home\", 1) = %d\n", file_manager.remove("/home", 1));
        printf("destroy() = %d\n\n", file_manager.destroy());
    }
    else
        printf("Memory allocating error!\n");

    if (current_directory != NULL)
        free(current_directory);

    return 0;
}