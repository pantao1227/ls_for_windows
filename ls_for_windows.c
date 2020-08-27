#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

struct StrListItem
{
    TCHAR content[MAX_PATH];
    struct StrListItem * next;
};

struct StrList
{
    int size;
    struct StrListItem * firstItem;
    struct StrListItem * lastItem;
};

int StrListAppend(struct StrList * p, TCHAR *str)
{
    struct StrListItem * pTmp = (struct StrListItem *)malloc(sizeof(struct StrListItem));
    strcpy(pTmp->content, str);
    pTmp->next = NULL;
    if( (p->firstItem == NULL) && (p->lastItem == NULL) && (p->size == 0) )
    {
        p->firstItem = pTmp;
        p->lastItem = pTmp;
    } else {
        p->lastItem->next = pTmp;
        p->lastItem = pTmp;
    }
    p->size++;

    return 0;
}

int StrListClear(struct StrList * p)
{
    struct StrListItem * pTmp;
    while(p->firstItem != NULL)
    {
        pTmp = p->firstItem;

        p->firstItem = p->firstItem->next;
        p->size--;

        pTmp->next = NULL;
        free(pTmp);
    }
    return 0;
}

int StrListPrint(struct StrList * p)
{
    struct StrListItem * pTmp = p->firstItem;
    while(1)
    {   if(pTmp == NULL)
            return 0;
        printf("%s\n", pTmp->content);
        if(pTmp->next == NULL)
            return 0;
        else
            pTmp = pTmp->next;
    }
    
}

TCHAR * GetContent(struct StrList * p, int n)
{
    int i;
    struct StrListItem * pTmp = p->firstItem;
    for(i=n; i>0; i--)
    {
        pTmp = pTmp->next;
    }
    return pTmp->content;
}

int main(int argc, char * argv[])
{
    if( (argc > 1) && !strcmp(argv[1], "/?") )
    {
        printf("Usage: ls [path]\n");
        return 0;
    }
    struct StrList dir_list = {0, NULL, NULL};
    struct StrList file_list = {0, NULL, NULL};

    DWORD dwRet;
    TCHAR root_dir_path_buffer[MAX_PATH];
    TCHAR dir_path_buffer[MAX_PATH];
    TCHAR temp[MAX_PATH];
    
    dwRet = GetCurrentDirectory(MAX_PATH, root_dir_path_buffer);
    if( dwRet == 0 )
    {
        printf("GetCurrentDirectory failed (%d)\n", GetLastError());
        return;
    }
    if( dwRet > MAX_PATH )
    {
        printf("Buffer too small; need %d characters\n", dwRet);
        return;
    }

    if(argc > 1)
    {
        if( !SetCurrentDirectory(argv[1]) )
        {
            printf("Set directory failed (%d)\n", GetLastError());
            return;
        }

        dwRet = GetCurrentDirectory(MAX_PATH, dir_path_buffer);
        if( dwRet == 0 )
        {
            printf("GetCurrentDirectory failed (%d)\n", GetLastError());
            return;
        }
        if( dwRet > MAX_PATH )
        {
            printf("Buffer too small; need %d characters\n", dwRet);
            return;
        }
    } else {
        strcpy(dir_path_buffer, root_dir_path_buffer);
    }

    WIN32_FIND_DATA wFd;
    HANDLE h;

    strcat(dir_path_buffer, "\\*");
    if(strlen(dir_path_buffer) > MAX_PATH)
        printf("Path length error.\n\r");

    int maxL = 0;

    h = FindFirstFile(dir_path_buffer, &wFd);
    if(h == INVALID_HANDLE_VALUE)
        return 0;

    while(FindNextFile(h, &wFd))
    {
        if(strcmp(wFd.cFileName, ".") && strcmp(wFd.cFileName, ".."))
            if(wFd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
            {
                strcpy(temp, "\033[1;36m");
                strcat(temp, wFd.cFileName);
                strcat(temp, "\033[0m");
                StrListAppend(&dir_list, temp);
                if(strlen(wFd.cFileName) > maxL)
                    maxL = strlen(wFd.cFileName);
            } else {
                strcpy(temp, "\033[1;92m");
                strcat(temp, wFd.cFileName);
                strcat(temp, "\033[0m");
                StrListAppend(&file_list, temp);
                if(strlen(wFd.cFileName) > maxL)
                    maxL = strlen(wFd.cFileName);
            }

    }

    struct StrList all_list = {0, NULL, NULL};
    if( (dir_list.firstItem != NULL) && (file_list.firstItem != NULL) )
    {
        all_list.size = dir_list.size + file_list.size;
        all_list.firstItem = dir_list.firstItem;
        all_list.lastItem = file_list.lastItem;
        dir_list.lastItem->next = file_list.firstItem;
    } else if( file_list.firstItem == NULL ) {
        all_list.size = dir_list.size;
        all_list.firstItem = dir_list.firstItem;
        all_list.lastItem = dir_list.lastItem;
    } else {
        all_list.size = file_list.size;
        all_list.firstItem = file_list.firstItem;
        all_list.lastItem = file_list.lastItem;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int console_columns, console_rows;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    console_columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    console_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    if(maxL >= console_columns)
    {
        StrListPrint(&all_list);
        goto end;
    }

    int cols, rows, width;
    width = maxL + 4;
    cols = console_columns / width;
    rows = (dir_list.size + file_list.size) / cols;
    if( (dir_list.size + file_list.size) % cols != 0 )
        rows++;

    int i,j;
    for(i = 0; i < rows; i++)
    {
        for(j = 0; j < cols; j++)
        {
            if(i + j*rows + 1 > all_list.size)
                break;
            printf("\033[%dD\033[%dC%s",console_columns, width*j, GetContent(&all_list, i + j * rows));
        }
        if( i != rows - 1 )
            printf("\n");
    }

end:
    StrListClear(&dir_list);

    if( !SetCurrentDirectory(root_dir_path_buffer) )
   {
      printf("SetCurrentDirectory failed (%d)\n", GetLastError());
      return;
   }

    return 0;
}