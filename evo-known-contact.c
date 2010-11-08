/*
 * evo-known-contact.c
 *
 * Copyright (C) 2004 Ross Burton
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Ross Burton <ross@burtonini.com
 */

#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <libebook/e-book.h>

static gboolean query_source (ESource *source, EBookQuery *query) {
  EBook *book;
  GList *contacts = NULL;
  gboolean retval = FALSE;
  book = e_book_new (source, NULL /* TODO */);
  e_book_open (book, TRUE, NULL /* TODO */);
  if (e_book_get_contacts (book, query, &contacts, NULL /* TODO */)) {
    if (g_list_length (contacts) > 0) {
      retval = TRUE;
    }
    g_list_foreach (contacts, (GFunc)g_object_unref, NULL);
    g_list_free (contacts);
  }
  g_object_unref (book);
  return retval;
}

static EBookQuery *build_query (char *email) {
  /* TODO: just search email fields */
  return e_book_query_any_field_contains (email);
}

int main(int argc, char **argv) {
  ESourceList *list = NULL;
  GIOChannel *io;
  gboolean reading = TRUE;
  char *email = NULL;
  EBookQuery *query;

  io = g_io_channel_unix_new (STDIN_FILENO);
  g_io_channel_set_encoding (io, NULL, NULL);
  while (reading) {
    char *line;
    switch (g_io_channel_read_line (io, &line, NULL, NULL, NULL)) {
    case G_IO_STATUS_ERROR:
      reading = FALSE;
      g_warning ("Got an error");
      break;
    case G_IO_STATUS_EOF:
      reading = FALSE;
      break;
    case G_IO_STATUS_NORMAL:
      if (strncmp (line, "From: ", 6) == 0) {
        char *name, *p;
        name = line + 6; /* "From: " */
        if ((p = strchr (name, '<'))) {
          name = p + 1;
          if ((p = strchr (name, '>'))) {
            *p = '\0';
          }
        }
        email = g_strdup (name);
        reading = FALSE;
      }
      g_free (line);
      break;
    case G_IO_STATUS_AGAIN:
      break;
    default:
      g_warning ("Unknown GIOStatus");
    }
  }

  if (email == NULL) {
    g_warning ("Could not find From: address");
    return 1;
  }

  query = build_query (email);

  if (e_book_get_addressbooks (&list, NULL /* TODO */)) {
    GSList *group_iter;
    for (group_iter = e_source_list_peek_groups (list); group_iter != NULL; group_iter = group_iter->next) {
      GSList *source_iter;
      ESourceGroup *group;
      group = E_SOURCE_GROUP (group_iter->data);
      for (source_iter = e_source_group_peek_sources (group); source_iter != NULL; source_iter = source_iter->next) {
        ESource *source;
        source = E_SOURCE (source_iter->data);
        if (query_source (source, query)) {
          return 0;
        }
      }
    }
  }

  return 1;
}
