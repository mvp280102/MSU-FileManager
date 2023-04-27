#include "os-file.h"

#include <string.h>
#include <stdio.h>


// Структура, представляющая файл/директорию:
typedef struct Node
{
    char name[33];                  // имя файла/директории
    unsigned int size;              // размер (для файлов)

    struct Node *parent;            // указатель на родительскую директорию

    struct Node **children;         // массив указателей на дочерние директории или файлы (для директорий)
    int children_amount;            // количество дочерних директорий или файлов (для директорий)
} Node;


// Прочие переменные:
unsigned int fm_size;               // размер диска
int created = 0;                    // флаг наличия файлового менеджера
Node *root_dir = NULL;              // указатель на корневую директорию
Node *cur_dir = NULL;               // указатель на текущую директорию


//// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ:

// Выделяет из строки подстроку от заданной позиции до указанного разделителя:
void split(const char *path, char *result, unsigned int *pos, const char divider)
{
    int i = 0;

    while ((path[*pos] != divider) && (*pos < strlen(path)))
        result[i++] = path[(*pos)++];

    result[i] = '\0';
    (*pos)++;
}

// Возвращает индекс элемента с заданным именем в массиве элементов
// типа Node или -1, если такого элемента в массиве не существует:
int bin_search(Node **arr, int arr_len, const char *name)
{
    int mid, left = 0, right = arr_len - 1;

    while (left <= right)
    {
        mid = (left + right) / 2;

        if (strcmp(name, arr[mid]->name) < 0)
            right = mid - 1;
        else if (strcmp(name, arr[mid]->name) > 0)
            left = mid + 1;
        else
            return mid;
    }

    return -1;
}

// Возвращает индекс, по которому нужно вставить новый элемент в
// массив элементов типа Node так, чтобы сохранить порядок сортировки
// массива или -1, если такой элемент в массиве уже существует:
int bin_insert(Node **arr, int arr_len, const char *name)
{
    int mid, left = 0, right = arr_len - 1;

    while (left < right)
    {
        mid = (left + right) / 2;

        if (strcmp(name, arr[mid]->name) < 0)
            if (left == right - 1) return left;
            else right = mid;
        else if (strcmp(name, arr[mid]->name) > 0)
            if (left == right - 1) return right;
            else left = mid;
        else
            return -1;
    }
}

// Определяет правильность заданной директории:
void exists(const char *path, Node **dst_dir)
{
    int sub_dir_index;
    unsigned int cur_pos;
    char *sub_dir = (char*)malloc(sizeof(char) * 33);

    if (sub_dir != NULL)
    {
        if (path[0] == '/')
        {
            *dst_dir = root_dir;
            cur_pos = 1;
        }
        else
        {
            *dst_dir = cur_dir;
            cur_pos = 0;
        }

        while (cur_pos < strlen(path))
        {
            split(path, sub_dir, &cur_pos, '/');

            if (strcmp(sub_dir, ".") == 0)
                continue;
            else if (strcmp(sub_dir, "..") == 0)
                if (*dst_dir == root_dir)
                {
                    *dst_dir = NULL;
                    break;
                }
                else
                    *dst_dir = (*dst_dir)->parent;
            else
            {
                if (!((*dst_dir)->size))
                {
                    sub_dir_index = bin_search((*dst_dir)->children, (*dst_dir)->children_amount, sub_dir);

                    if (sub_dir_index == -1)
                    {
                        *dst_dir = NULL;
                        break;
                    }
                    else
                        *dst_dir = (*dst_dir)->children[sub_dir_index];
                }
                else
                {
                    *dst_dir = NULL;
                    break;
                }
            }
        }
    }
    else
        *dst_dir = NULL;

    if (sub_dir != NULL)
        free(sub_dir);
}

// В зависимости от указанного размера создает файл или директорию:
int create_node(const char *path, int size)
{
    Node *temp_dir;
    int err_code = 1, req_pos;
    unsigned int len = strlen(path);
    unsigned int cur_pos = 0;
    char *dst_dir = (char*)malloc(sizeof(char) * 33),
         *temp_path = (char*)malloc(sizeof(char) * strlen(path));

    if ((dst_dir != NULL) && (temp_path != NULL))
    {
        strcpy(temp_path, "");

        while (cur_pos < len)
        {
            strcpy(dst_dir, "");
            split(path, dst_dir, &cur_pos, '/');

            if (cur_pos >= len)
                break;

            strcat(temp_path, dst_dir);
            strcat(temp_path, "/");
        }

        exists(temp_path, &temp_dir);

        if ((strcmp(dst_dir, ".") != 0) && (strcmp(dst_dir, "..") != 0) && (temp_dir != NULL) &&
            (!temp_dir->size) && (bin_search(temp_dir->children, temp_dir->children_amount, dst_dir) == -1))
        {
            if (temp_dir->children_amount)
            {
                temp_dir->children = (Node**)realloc(temp_dir->children, sizeof(Node*) * (temp_dir->children_amount + 1));

                if (temp_dir->children != NULL)
                {
                    temp_dir->children_amount++;
                    req_pos = bin_insert(temp_dir->children, temp_dir->children_amount, dst_dir);

                    for (int i = temp_dir->children_amount - 1; i > req_pos; i--)
                        temp_dir->children[i] = temp_dir->children[i - 1];

                    temp_dir->children[req_pos] = (Node*)malloc(sizeof(Node));

                    if (temp_dir->children[req_pos] != NULL)
                    {
                        strcpy(temp_dir->children[req_pos]->name, dst_dir);
                        temp_dir->children[req_pos]->size = size;
                        temp_dir->children[req_pos]->parent = temp_dir;
                        temp_dir->children[req_pos]->children = NULL;
                        temp_dir->children[req_pos]->children_amount = 0;

                        fm_size -= size;
                    }
                    else
                        err_code = 0;
                }
                else
                    err_code = 0;
            }
            else
            {
                temp_dir->children = (Node**)malloc(sizeof(Node*));

                if (temp_dir->children != NULL)
                {
                    temp_dir->children[0] = (Node*)malloc(sizeof(Node));

                    if (temp_dir->children[0] != NULL)
                    {
                        temp_dir->children_amount++;

                        strcpy(temp_dir->children[0]->name, dst_dir);
                        temp_dir->children[0]->size = size;
                        temp_dir->children[0]->parent = temp_dir;
                        temp_dir->children[0]->children = NULL;
                        temp_dir->children[0]->children_amount = 0;

                        fm_size -= size;
                    }
                    else
                        err_code = 0;
                }
                else
                    err_code = 0;
            }
        }
        else
            err_code = 0;
    }
    else
        err_code = 0;

    if (dst_dir != NULL)
        free(dst_dir);

    if (temp_path != NULL)
        free(temp_path);

    return err_code;
}

// Определяет путь до заданной директории:
void get_cur_path(Node *dir, char *dst)
{
    if (strcmp(dir->name,  "/") == 0)
        strcpy(dst, "/");
    else
    {
        get_cur_path(dir->parent, dst);
        strcat(dst, dir->name);
        strcat(dst, "/");
    }
}


// ОСНОВНЫЕ ФУНКЦИИ:

// Создает файловый менеджер:
int create(int disk_size)
{
    int err_code = 1;

    if (!created)
    {
        root_dir = (Node*)malloc(sizeof(Node));

        if (root_dir != NULL)
        {
            strcpy(root_dir->name, "/");
            root_dir->size = 0;
            root_dir->parent = NULL;
            root_dir->children = NULL;
            root_dir->children_amount = 0;

            fm_size = disk_size;
            created = 1;

            cur_dir = root_dir;
        }
        else
            err_code = 0;
    }
    else
        err_code = 0;

    return err_code;
}

// Создает директорию с указанным именем:
int create_dir(const char *path)
{
    int err_code;

    if (created)
        err_code = create_node(path, 0);
    else
        err_code = 0;

    return err_code;
}

// Создает файл с указанными именем и размером:
int create_file(const char *path, int file_size)
{
    int err_code;

    if (created)
        if (file_size <= fm_size)
            err_code = create_node(path, file_size);
        else
            err_code = 0;
    else
        err_code = 0;

    return err_code;
}

// Удаляет директорию или файл с указанным именем:
int remove_dir_file(const char *path, int recursive)
{
    Node *temp_dir, *temp_cur_dir = cur_dir;
    int err_code = 1, req_pos;

    exists(path, &temp_dir);

    if (created && (temp_dir != NULL) && (strcmp(path, "/") != 0))
    {
        if ((temp_dir->size) || (!temp_dir->children_amount))
        {
            req_pos = bin_search(temp_dir->parent->children, temp_dir->parent->children_amount, temp_dir->name);

            temp_dir = temp_dir->parent;
            fm_size += temp_dir->children[req_pos]->size;

            if (strcmp(temp_dir->children[req_pos]->name, cur_dir->name) == 0)
                cur_dir = root_dir;

            free(temp_dir->children[req_pos]);

            for (int i = req_pos; i < temp_dir->children_amount - 1; i++)
                temp_dir->children[i] = temp_dir->children[i + 1];

            temp_dir->children_amount--;

            if (!temp_dir->children_amount)
            {
                free(temp_dir->children);
                temp_dir->children = NULL;
            }
            else
                temp_dir->children = (Node**)realloc(temp_dir->children, sizeof(Node*) * temp_dir->children_amount);
        }
        else
        {
            if (recursive)
            {
                cur_dir = temp_dir;

                for (int i = 0; i < temp_dir->children_amount; i++)
                    err_code = remove_dir_file(temp_dir->children[i]->name, 1);

                cur_dir = temp_cur_dir;

                err_code = remove_dir_file(path, 0);
            }
            else
                err_code = 0;
        }
    }
    else
        err_code = 0;

    return err_code;
}

// Удаляет файловый менеджер:
int destroy()
{
    int err_code = 1;

    if (created)
    {
        cur_dir = root_dir;

        while (root_dir->children_amount)
            err_code = remove_dir_file(root_dir->children[0]->name, 1);

        free(root_dir->children);
        free(root_dir);
        created = 0;
    }
    else
        err_code = 0;

    return err_code;
}

// Меняет текущую директорию на заданную:
int change_dir(const char *path)
{
    Node *temp_dir;
    int err_code = 1;

    exists(path, &temp_dir);

    if (temp_dir != NULL)
        cur_dir = temp_dir;
    else
        err_code = 0;

    return err_code;
}

// Возвращает текущую директорию:
void get_cur_dir(char *dst)
{
    get_cur_path(cur_dir, dst);
}

// Для директории выводит на экран ее содержимое. Для файла - его путь:
int list(const char *path, int dir_first)
{
    Node *temp_dir;
    int err_code = 1;
    unsigned int len = strlen(path);
    unsigned int cur_pos_before = 0, cur_pos_after = 0;
    char *temp_path;

    while (cur_pos_after < len)
    {
        while ((path[cur_pos_after] != ' ') && (cur_pos_after < len))
            cur_pos_after++;

        cur_pos_after++;

        temp_path = (char*)malloc(sizeof(char) * (cur_pos_after - cur_pos_before));
        split(path, temp_path, &cur_pos_before, ' ');

        if (temp_path != NULL)
        {
            exists(temp_path, &temp_dir);

            if (temp_dir != NULL)
            {
                if (temp_dir->size)
                    printf("%s\n", temp_path);
                else
                {
                    printf("%s:\n", temp_path);

                    if (temp_dir->children_amount)
                    {
                        if (dir_first)
                        {
                            for (int i = 0; i < temp_dir->children_amount; i++)
                                if (!temp_dir->children[i]->size)
                                    printf("%s\n", temp_dir->children[i]->name);

                            for (int i = 0; i < temp_dir->children_amount; i++)
                                if (temp_dir->children[i]->size)
                                    printf("%s\n", temp_dir->children[i]->name);
                        }
                        else
                            for (int i = 0; i < temp_dir->children_amount; i++)
                                printf("%s\n", temp_dir->children[i]->name);
                    }
                    else
                        printf("\n");
                }
            }
            else
                err_code = 0;
        }
        else
            err_code = 0;

        if (temp_path != NULL)
            free(temp_path);
    }

    return err_code;
}

// Настаивает файловый менеджер перед началом работы:
void setup_file_manager(file_manager_t *fm)
{
    fm->create = create;
    fm->destroy = destroy;
    fm->create_dir = create_dir;
    fm->create_file = create_file;
    fm->remove = remove_dir_file;
    fm->change_dir = change_dir;
    fm->get_cur_dir = get_cur_dir;
    fm->list = list;
}