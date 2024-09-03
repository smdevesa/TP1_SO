//
// Created by Santiago Devesa on 03/09/2024.
//

#ifndef TP1_SO_SLAVE_H
#define TP1_SO_SLAVE_H

#define PROGRAM "md5sum"
#define INITIAL_LEN 7

#define MALLOC_ERROR "[slave] malloc error\n"
#define POPEN_ERROR "[slave] pipe creation error\n"
#define GETLINE_ERROR "[slave] getline error\n"

/**
 * @brief Joins the PROGRAM constant with the filePath parameter.
 * @param filePath
 * @return The concatenated string or NULL if there is a memory error.
 */
char * concatenatePath(char * filePath);

#endif //TP1_SO_SLAVE_H
