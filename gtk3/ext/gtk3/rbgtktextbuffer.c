/* -*- c-file-style: "ruby"; indent-tabs-mode: nil -*- */
/*
 *  Copyright (C) 2011  Ruby-GNOME2 Project Team
 *  Copyright (C) 2002-2005 Ruby-GNOME2 Project Team
 *  Copyright (C) 2002,2003 Masahiro Sakai
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#include "rbgtk3private.h"

#define RG_TARGET_NAMESPACE cTextBuffer
#define _SELF(s) (RVAL2GTKTEXTBUFFER(s))

static VALUE rb_mGtk;

#define RVAL2ITER(self, position) RVAL2GTKTEXTITER(rg_get_iter_at(self, position))
#define RVAL2STARTITER(self, iter, out) \
        rval2iter_with_default(&(self), &(iter), &(out), gtk_text_buffer_get_start_iter)
#define RVAL2ENDITER(self, iter, out) \
        rval2iter_with_default(&(self), &(iter), &(out), gtk_text_buffer_get_end_iter)

static VALUE
rg_get_iter_at(VALUE self, VALUE position)
{
    GtkTextIter iter;

    switch (TYPE(position)) {
      case T_HASH:
      {
        VALUE line, offset, index, mark, anchor;
        rbg_scan_options(position,
                         "line", &line,
                         "offset", &offset,
                         "index", &index,
                         "mark", &mark,
                         "anchor", &anchor,
                         NULL);

        if (!NIL_P(line))
            if (!NIL_P(offset))
                gtk_text_buffer_get_iter_at_line_offset(_SELF(self),
                                                        &iter,
                                                        NUM2INT(line),
                                                        NUM2INT(offset));
            else if (!NIL_P(index))
                gtk_text_buffer_get_iter_at_line_index(_SELF(self),
                                                       &iter,
                                                       NUM2INT(line),
                                                       NUM2INT(index));
            else
                gtk_text_buffer_get_iter_at_line(_SELF(self),
                                                 &iter,
                                                 NUM2INT(line));
        else if (!NIL_P(offset))
            gtk_text_buffer_get_iter_at_offset(_SELF(self),
                                               &iter,
                                               NUM2INT(offset));
        else if (!NIL_P(mark))
            gtk_text_buffer_get_iter_at_mark(_SELF(self),
                                             &iter,
                                             RVAL2GTKTEXTMARK(mark));
        else if (!NIL_P(anchor))
            gtk_text_buffer_get_iter_at_child_anchor(_SELF(self),
                                                     &iter,
                                                     RVAL2GTKTEXTCHILDANCHOR(anchor));
        else
            rb_raise(rb_eArgError, "Invalid arguments.");
        break;
      }
      case T_FIXNUM:
      {
        gtk_text_buffer_get_iter_at_offset(_SELF(self),
                                           &iter,
                                           NUM2INT(position));
        break;
      }
      default:
      {
        GType gtype = RVAL2GTYPE(position);

        if (g_type_is_a(gtype, GTK_TYPE_TEXT_ITER))
            return position;
        else if (g_type_is_a(gtype, GTK_TYPE_TEXT_MARK))
            gtk_text_buffer_get_iter_at_mark(_SELF(self),
                                             &iter,
                                             RVAL2GTKTEXTMARK(position));
        else if (g_type_is_a(gtype, GTK_TYPE_TEXT_CHILD_ANCHOR))
            gtk_text_buffer_get_iter_at_child_anchor(_SELF(self),
                                                     &iter,
                                                     RVAL2GTKTEXTCHILDANCHOR(position));
        else
            rb_raise(rb_eArgError, "Invalid arguments.");
        break;
      }
    }

    return GTKTEXTITER2RVAL(&iter);
}

static GtkTextIter *
rval2iter_with_default(VALUE *self, VALUE *iter, GtkTextIter *out,
                       void (*default_func)(GtkTextBuffer *, GtkTextIter *))
{
    if (NIL_P(*iter)) {
        default_func(_SELF(*self), out);
        return out;
    } else {
        return RVAL2ITER(*self, *iter);
    }
}

static VALUE
rg_initialize(int argc, VALUE *argv, VALUE self)
{
    VALUE table;
    rb_scan_args(argc, argv, "01", &table);
    if (NIL_P(table))
        G_INITIALIZE(self, gtk_text_buffer_new(NULL));
    else {
        G_CHILD_SET(self, rb_intern("tagtable"), table);
        G_INITIALIZE(self, gtk_text_buffer_new(RVAL2GTKTEXTTAGTABLE(table)));
    }
    return Qnil;
}

static VALUE
rg_line_count(VALUE self)
{
    return INT2NUM(gtk_text_buffer_get_line_count(_SELF(self)));
}

static VALUE
rg_char_count(VALUE self)
{
    return INT2NUM(gtk_text_buffer_get_char_count(_SELF(self)));
}

static VALUE
txt_set_text(VALUE self, VALUE text)
{
    StringValue(text);
    gtk_text_buffer_set_text(_SELF(self), RSTRING_PTR(text), RSTRING_LEN(text));
    return self;
}

static VALUE
rg_backspace(VALUE self, VALUE iter, VALUE interactive, VALUE default_editable)
{
    return CBOOL2RVAL(gtk_text_buffer_backspace(_SELF(self), RVAL2ITER(self, iter),
                                                RVAL2CBOOL(interactive),
                                                RVAL2CBOOL(default_editable)));
}

static VALUE
rg_insert_at_cursor(VALUE self, VALUE text)
{
    StringValue(text);
    gtk_text_buffer_insert_at_cursor(_SELF(self), RSTRING_PTR(text), RSTRING_LEN(text));
    return self;
}

static VALUE
rg_insert_interactive(VALUE self, VALUE iter, VALUE text, VALUE editable)
{
    StringValue(text);

    return CBOOL2RVAL(gtk_text_buffer_insert_interactive(_SELF(self),
                                                         RVAL2ITER(self, iter),
                                                         RSTRING_PTR(text),
                                                         RSTRING_LEN(text),
                                                         RVAL2CBOOL(editable)));
}

static VALUE
rg_insert_interactive_at_cursor(VALUE self, VALUE text, VALUE editable)
{
    StringValue(text);

    return CBOOL2RVAL(gtk_text_buffer_insert_interactive_at_cursor(_SELF(self),
                                                                   RSTRING_PTR(text),
                                                                   RSTRING_LEN(text),
                                                                   RVAL2CBOOL(editable)));
}

static VALUE
rg_insert_range(VALUE self, VALUE iter, VALUE start, VALUE end)
{
    gtk_text_buffer_insert_range(_SELF(self), RVAL2ITER(self, iter),
                                 RVAL2ITER(self, start), RVAL2ITER(self, end));
    return self;
}

static VALUE
rg_insert_range_interactive(VALUE self, VALUE iter, VALUE start, VALUE end, VALUE editable)
{
    return CBOOL2RVAL(gtk_text_buffer_insert_range_interactive(_SELF(self),
                                                               RVAL2ITER(self, iter),
                                                               RVAL2ITER(self, start),
                                                               RVAL2ITER(self, end),
                                                               RVAL2CBOOL(editable)));
}

static VALUE
rg_delete(int argc, VALUE *argv, VALUE self)
{
    VALUE start, end;
    GtkTextIter start_iter, end_iter;

    rb_scan_args(argc, argv, "02", &start, &end);

    gtk_text_buffer_delete(_SELF(self),
                           RVAL2STARTITER(self, start, start_iter),
                           RVAL2ENDITER(self, end, end_iter));

    return self;
}

static VALUE
rg_delete_interactive(int argc, VALUE *argv, VALUE self)
{
    VALUE start, end, editable;
    GtkTextIter start_iter, end_iter;

    rb_scan_args(argc, argv, "03", &start, &end, &editable);

    return CBOOL2RVAL(gtk_text_buffer_delete_interactive(_SELF(self),
                                                         RVAL2STARTITER(self, start, start_iter),
                                                         RVAL2ENDITER(self, end, end_iter),
                                                         RVAL2CBOOL(editable)));
}

static VALUE
rg_get_text(int argc, VALUE *argv, VALUE self)
{
    VALUE start, end, include_hidden_chars;
    GtkTextIter start_iter, end_iter;
    gchar* ret;

    rb_scan_args(argc, argv, "03", &start, &end, &include_hidden_chars);

    ret = gtk_text_buffer_get_text(_SELF(self),
                                   RVAL2STARTITER(self, start, start_iter),
                                   RVAL2ENDITER(self, end, end_iter),
                                   RVAL2CBOOL(include_hidden_chars));

    return CSTR2RVAL_FREE(ret);
}

static VALUE
txt_get_text_all(VALUE self)
{
    return rg_get_text(0, NULL, self);
}

static VALUE
rg_get_slice(int argc, VALUE *argv, VALUE self)
{
    VALUE start, end, include_hidden_chars;
    GtkTextIter start_iter, end_iter;
    gchar* ret;

    rb_scan_args(argc, argv, "03", &start, &end, &include_hidden_chars);

    ret = gtk_text_buffer_get_slice(_SELF(self),
                                    RVAL2STARTITER(self, start, start_iter),
                                    RVAL2ENDITER(self, end, end_iter),
                                    RVAL2CBOOL(include_hidden_chars));

    return CSTR2RVAL_FREE(ret);
}

static VALUE
rg_slice(VALUE self)
{
    return rg_get_slice(0, NULL, self);
}

static VALUE
rg_create_child_anchor(VALUE self, VALUE iter)
{
    VALUE ret = GOBJ2RVAL(gtk_text_buffer_create_child_anchor(_SELF(self), RVAL2ITER(self, iter)));
    G_CHILD_ADD(self, ret);
    return ret;
}

static VALUE
rg_create_mark(VALUE self, VALUE name, VALUE where, VALUE left_gravity)
{
    VALUE ret = GOBJ2RVAL(gtk_text_buffer_create_mark(_SELF(self),
                                                      RVAL2CSTR_ACCEPT_NIL(name),
                                                      RVAL2ITER(self, where),
                                                      RVAL2CBOOL(left_gravity)));
    G_CHILD_ADD(self, ret);
    return ret;
}

static VALUE
rg_add_mark(VALUE self, VALUE mark, VALUE where)
{
    gtk_text_buffer_add_mark(_SELF(self), RVAL2GTKTEXTMARK(mark), RVAL2ITER(self, where));
    return self;
}

static VALUE
rg_delete_mark(VALUE self, VALUE mark)
{
    if (rb_obj_is_kind_of(mark, GTYPE2CLASS(GTK_TYPE_TEXT_MARK))){
        G_CHILD_REMOVE(self, mark);
        gtk_text_buffer_delete_mark(_SELF(self), RVAL2GTKTEXTMARK(mark));
    } else {
        G_CHILD_REMOVE(self, GOBJ2RVAL(gtk_text_buffer_get_mark(_SELF(self), RVAL2CSTR(mark))));
        gtk_text_buffer_delete_mark_by_name(_SELF(self), RVAL2CSTR(mark));
    }
    return self;
}

static VALUE
rg_get_mark(VALUE self, VALUE name)
{
    return GOBJ2RVAL(gtk_text_buffer_get_mark(_SELF(self), RVAL2CSTR(name)));
}

/*
static VALUE
rg_get_insert(VALUE self)
{
    return GOBJ2RVAL(gtk_text_buffer_get_insert(_SELF(self)));
}
*/

static VALUE
rg_selection_bound(VALUE self)
{
    return GOBJ2RVAL(gtk_text_buffer_get_selection_bound(_SELF(self)));
}

static VALUE
rg_place_cursor(VALUE self, VALUE where)
{
    gtk_text_buffer_place_cursor(_SELF(self), RVAL2ITER(self, where));
    return self;
}

static VALUE
rg_select_range(VALUE self, VALUE ins, VALUE bound)
{
    gtk_text_buffer_select_range(_SELF(self), RVAL2ITER(self, ins), RVAL2ITER(self, bound));
    return self;
}

static VALUE
rg_modified_p(VALUE self)
{
    return CBOOL2RVAL(gtk_text_buffer_get_modified(_SELF(self)));
}

static VALUE
rg_set_modified(VALUE self, VALUE setting)
{
    gtk_text_buffer_set_modified(_SELF(self), RVAL2CBOOL(setting));
    return setting;
}

static VALUE
rg_add_selection_clipboard(VALUE self, VALUE clipboard)
{
    G_CHILD_ADD(self, clipboard);
    gtk_text_buffer_add_selection_clipboard(_SELF(self), RVAL2GTKCLIPBOARD(clipboard));
    return self;
}

static VALUE
rg_remove_selection_clipboard(VALUE self, VALUE clipboard)
{
    G_CHILD_REMOVE(self, clipboard);
    gtk_text_buffer_remove_selection_clipboard(_SELF(self), RVAL2GTKCLIPBOARD(clipboard));
    return self;
}

static VALUE
rg_deserialize(VALUE self, VALUE content_buffer, VALUE format, VALUE iter, VALUE data)
{
    GError* error = NULL;
    gboolean ret;

    StringValue(data);
    ret = gtk_text_buffer_deserialize(_SELF(self), _SELF(content_buffer),
                                      RVAL2ATOM(format),
                                      RVAL2ITER(self, iter),
                                      (const guint8*)RSTRING_PTR(data),
                                      (gsize)RSTRING_LEN(data),
                                      &error);
    if (! ret) RAISE_GERROR(error);
    return self;
}

static VALUE
rg_deserialize_can_create_tags_p(VALUE self, VALUE format)
{
    return CBOOL2RVAL(gtk_text_buffer_deserialize_get_can_create_tags(_SELF(self), 
                                                                      RVAL2ATOM(format)));

}

static VALUE
rg_deserialize_set_can_create_tags(VALUE self, VALUE format, VALUE can_create_tags)
{
    gtk_text_buffer_deserialize_set_can_create_tags(_SELF(self),
                                                    RVAL2ATOM(format),
                                                    RVAL2CBOOL(can_create_tags));
    return self;
}

static VALUE
rg_deserialize_formats(VALUE self)
{
    gint i;
    gint n_formats;
    GdkAtom* atoms = gtk_text_buffer_get_deserialize_formats(_SELF(self), &n_formats);
    VALUE ary = rb_ary_new();

    for (i = 0; i < n_formats; i++){
        rb_ary_push(ary, GDKATOM2RVAL(atoms[i]));
    }
    return ary;
}

static VALUE
rg_serialize_formats(VALUE self)
{
    gint i;
    gint n_formats;
    GdkAtom* atoms = gtk_text_buffer_get_serialize_formats(_SELF(self), &n_formats);
    VALUE ary = rb_ary_new();

    for (i = 0; i < n_formats; i++){
        rb_ary_push(ary, GDKATOM2RVAL(atoms[i]));
    }
    return ary;
}

struct callback_arg
{
    VALUE callback;
    int argc;
    VALUE *argv;
};

static VALUE
invoke_callback(VALUE data)
{
    struct callback_arg *arg = (struct callback_arg *)data;

    return rb_funcall2(arg->callback, id_call, arg->argc, arg->argv);
}

static gboolean
deserialize_func(GtkTextBuffer *register_buffer,
                 GtkTextBuffer *content_buffer,
                 GtkTextIter *iter,
                 const guint8 *data,
                 gsize length,
                 gboolean create_tags,
                 gpointer func,
                 G_GNUC_UNUSED GError **error)
{
    VALUE result;
    VALUE argv[5];
    struct callback_arg arg;

    argv[0] = GOBJ2RVAL(register_buffer);
    argv[1] = GOBJ2RVAL(content_buffer);
    argv[2] = GTKTEXTITER2RVAL(iter);
    argv[3] = RBG_STRING_SET_UTF8_ENCODING(rb_str_new((char*)data, length));
    argv[4] = CBOOL2RVAL(create_tags);

    arg.callback = (VALUE)func;
    arg.argc = 5;
    arg.argv = argv;

    result = G_PROTECT_CALLBACK(invoke_callback, &arg);
    return NIL_P(rb_errinfo()) ? RVAL2CBOOL(result) : FALSE;
}

static void
remove_callback_reference(gpointer callback)
{
    G_CHILD_REMOVE(rb_mGtk, (VALUE)callback);
}

static VALUE
rg_register_deserialize_format(VALUE self, VALUE mime_type)
{
    VALUE block = rb_block_proc();
    GdkAtom atom;
    G_CHILD_ADD(rb_mGtk, block);
    atom = gtk_text_buffer_register_deserialize_format(_SELF(self),
                                                       (const gchar*)RVAL2CSTR(mime_type),
                                                       (GtkTextBufferDeserializeFunc)deserialize_func,
                                                       (gpointer)block,
                                                       (GDestroyNotify)remove_callback_reference);
    return GDKATOM2RVAL(atom);
}

static VALUE
rg_register_deserialize_target(VALUE self, VALUE tagset_name)
{
    return GDKATOM2RVAL(gtk_text_buffer_register_deserialize_tagset(_SELF(self),
                                                                 RVAL2CSTR_ACCEPT_NIL(tagset_name)));
}

static guint8*
serialize_func(GtkTextBuffer *register_buffer, GtkTextBuffer *content_buffer, GtkTextIter *start, GtkTextIter *end, gsize *length, gpointer func)
{
    VALUE result;
    VALUE argv[4];
    struct callback_arg arg;

    argv[0] = GOBJ2RVAL(register_buffer);
    argv[1] = GOBJ2RVAL(content_buffer);
    argv[2] = GTKTEXTITER2RVAL(start);
    argv[3] = GTKTEXTITER2RVAL(end);

    arg.callback = (VALUE)func;
    arg.argc = 4;
    arg.argv = argv;

    /* This should return data as String */
    result = G_PROTECT_CALLBACK(invoke_callback, &arg);
    StringValue(result);
    *length = RSTRING_LEN(result);
    return (guint8*)(NIL_P(rb_errinfo()) ? RSTRING_PTR(result) : NULL);
}

static VALUE
rg_register_serialize_format(VALUE self, VALUE mime_type)
{
    VALUE block = rb_block_proc();
    GdkAtom atom;
    G_CHILD_ADD(rb_mGtk, block);
    atom = gtk_text_buffer_register_serialize_format(_SELF(self),
                                                     (const gchar*)RVAL2CSTR(mime_type),
                                                     (GtkTextBufferSerializeFunc)serialize_func,
                                                     (gpointer)block,
                                                     (GDestroyNotify)remove_callback_reference);
    return GDKATOM2RVAL(atom);
}

static VALUE
rg_register_serialize_target(VALUE self, VALUE tagset_name)
{
    return GDKATOM2RVAL(gtk_text_buffer_register_serialize_tagset(_SELF(self),
                                                               RVAL2CSTR_ACCEPT_NIL(tagset_name)));
}

static VALUE
rg_serialize(VALUE self, VALUE content_buffer, VALUE format, VALUE start, VALUE end)
{
    gsize length;
    guint8* ret = gtk_text_buffer_serialize(_SELF(self), _SELF(content_buffer),
                                            RVAL2ATOM(format),
                                            RVAL2ITER(self, start), RVAL2ITER(self, end),
                                            &length);
    return RBG_STRING_SET_UTF8_ENCODING(rb_str_new((char*)ret, length));
}

static VALUE
rg_unregister_deserialize_format(VALUE self, VALUE format)
{
    gtk_text_buffer_unregister_deserialize_format(_SELF(self), RVAL2ATOM(format));
    return self;
}

static VALUE
rg_unregister_serialize_format(VALUE self, VALUE format)
{
    gtk_text_buffer_unregister_serialize_format(_SELF(self), RVAL2ATOM(format));
    return self;
}

static VALUE
rg_cut_clipboard(VALUE self, VALUE clipboard, VALUE default_editable)
{
    G_CHILD_ADD(self, clipboard);
    gtk_text_buffer_cut_clipboard(_SELF(self), RVAL2GTKCLIPBOARD(clipboard), RVAL2CBOOL(default_editable));
    return self;
}

static VALUE
rg_copy_clipboard(VALUE self, VALUE clipboard)
{
    G_CHILD_ADD(self, clipboard);
    gtk_text_buffer_copy_clipboard(_SELF(self), RVAL2GTKCLIPBOARD(clipboard));
    return self;
}

static VALUE
rg_paste_clipboard(VALUE self, VALUE clipboard, VALUE location, VALUE default_editable)
{
    G_CHILD_ADD(self, clipboard);
    gtk_text_buffer_paste_clipboard(_SELF(self), RVAL2GTKCLIPBOARD(clipboard),
                                    NIL_P(location) ? NULL : RVAL2ITER(self, location),
                                    RVAL2CBOOL(default_editable));
    return self;
}

static VALUE
rg_selection_bounds(VALUE self)
{
    GtkTextIter start, end;

    gboolean ret = gtk_text_buffer_get_selection_bounds(_SELF(self), &start, &end);
    return rb_ary_new3(3, GTKTEXTITER2RVAL(&start), GTKTEXTITER2RVAL(&end), CBOOL2RVAL(ret));
}

static VALUE
rg_delete_selection(int argc, VALUE *argv, VALUE self)
{
    VALUE interactive, default_editable;
    rb_scan_args(argc, argv, "20", &interactive, &default_editable); 
    return CBOOL2RVAL(gtk_text_buffer_delete_selection(_SELF(self),
                                                       RVAL2CBOOL(interactive), RVAL2CBOOL(default_editable)));
}

static VALUE
rg_end_user_action(VALUE self)
{
    gtk_text_buffer_end_user_action(_SELF(self));
    return self;
}

static VALUE
rg_begin_user_action(VALUE self)
{
    gtk_text_buffer_begin_user_action(_SELF(self));
    if (rb_block_given_p())
        rb_ensure(rb_yield, self, rg_end_user_action, self);
    return self;
}

static VALUE
rg_start_iter(VALUE self)
{
    GtkTextIter iter;
    gtk_text_buffer_get_start_iter(_SELF(self), &iter);
    return GTKTEXTITER2RVAL(&iter);
}

static VALUE
rg_end_iter(VALUE self)
{
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(_SELF(self), &iter);
    return GTKTEXTITER2RVAL(&iter);
}

static VALUE
rg_move_mark(VALUE self, VALUE mark, VALUE where)
{
    if (rb_obj_is_kind_of(mark, GTYPE2CLASS(GTK_TYPE_TEXT_MARK)))
        gtk_text_buffer_move_mark(_SELF(self), RVAL2GTKTEXTMARK(mark), RVAL2ITER(self, where));
    else
        gtk_text_buffer_move_mark_by_name(_SELF(self), RVAL2CSTR(mark), RVAL2ITER(self, where));
    return self;
}

static VALUE
rg_create_tag(VALUE self, VALUE tag_name, VALUE properties)
{
    GtkTextTag *tag;
    VALUE ret;

    tag = gtk_text_tag_new(RVAL2CSTR_ACCEPT_NIL(tag_name));
    gtk_text_tag_table_add (gtk_text_buffer_get_tag_table(_SELF(self)), tag);

    G_SET_PROPERTIES(GOBJ2RVAL(tag), properties);

    ret = GOBJ2RVAL(tag);

    G_CHILD_ADD(self, ret);

    return ret;
}

static VALUE
rg_insert(int argc, VALUE *argv, VALUE self)
{
    VALUE where, value, tags;
    gint start_offset;
    GtkTextIter start;
    int i;

    rb_scan_args(argc, argv, "2*", &where, &value, &tags);

    G_CHILD_ADD(self, where);
    G_CHILD_ADD(self, value);
    if (rb_obj_is_kind_of(value, GTYPE2CLASS(GDK_TYPE_PIXBUF))){
        gtk_text_buffer_insert_pixbuf(_SELF(self), RVAL2ITER(self, where),
                                      RVAL2GDKPIXBUF(value));
    } else if (rb_obj_is_kind_of(value, GTYPE2CLASS(GTK_TYPE_TEXT_CHILD_ANCHOR))){
        gtk_text_buffer_insert_child_anchor(_SELF(self), RVAL2ITER(self, where),
                                            RVAL2GTKTEXTCHILDANCHOR(value));
    } else {
        start_offset = gtk_text_iter_get_offset(RVAL2ITER(self, where));
        StringValue(value);
        gtk_text_buffer_insert(_SELF(self), RVAL2ITER(self, where),
                               RSTRING_PTR(value), RSTRING_LEN(value));

        if (RARRAY_LEN(tags) == 0)
            return self;

        /* TODO: Do we really want to do this before we know that everything
         * below passed without any errors? */
        G_CHILD_ADD(self, tags);

        gtk_text_buffer_get_iter_at_offset(_SELF(self), &start, start_offset);

        for(i = 0; i < RARRAY_LEN(tags); i++) {
            GtkTextTag *tag;

            if (rb_obj_is_kind_of(RARRAY_PTR(tags)[i], GTYPE2CLASS(GTK_TYPE_TEXT_TAG))) {
                tag = RVAL2GOBJ(RARRAY_PTR(tags)[i]);
            } else {
                tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(_SELF(self)),
                                                RVAL2CSTR(RARRAY_PTR(tags)[i]));
                if (tag == NULL) {
                    g_warning ("%s: no tag with name '%s'!",
                               G_STRLOC, RVAL2CSTR(RARRAY_PTR(tags)[i]));
                    return self;
                }
            }
            gtk_text_buffer_apply_tag(_SELF(self), tag, &start, RVAL2ITER(self, where));
        }
    }
    return self;
}

static VALUE
rg_apply_tag(int argc, VALUE *argv, VALUE self)
{
    VALUE tag, start, end;
    GtkTextIter start_iter, end_iter;

    rb_scan_args(argc, argv, "12", &tag, &start, &end);

    if (rb_obj_is_kind_of(tag, GTYPE2CLASS(GTK_TYPE_TEXT_TAG)))
        gtk_text_buffer_apply_tag(_SELF(self),
                                  RVAL2GTKTEXTTAG(tag),
                                  RVAL2STARTITER(self, start, start_iter),
                                  RVAL2ENDITER(self, end, end_iter));
    else
        gtk_text_buffer_apply_tag_by_name(_SELF(self),
                                          RVAL2CSTR(tag),
                                          RVAL2STARTITER(self, start, start_iter),
                                          RVAL2ENDITER(self, end, end_iter));

    return self;
}

static VALUE
rg_remove_tag(int argc, VALUE *argv, VALUE self)
{
    VALUE tag, start, end;
    GtkTextIter start_iter, end_iter;

    rb_scan_args(argc, argv, "12", &tag, &start, &end);

    if (rb_obj_is_kind_of(tag, GTYPE2CLASS(GTK_TYPE_TEXT_TAG)))
        gtk_text_buffer_remove_tag(_SELF(self),
                                   RVAL2GTKTEXTTAG(tag),
                                   RVAL2STARTITER(self, start, start_iter),
                                   RVAL2ENDITER(self, end, end_iter));
    else
        gtk_text_buffer_remove_tag_by_name(_SELF(self),
                                           RVAL2CSTR(tag),
                                           RVAL2STARTITER(self, start, start_iter),
                                           RVAL2ENDITER(self, end, end_iter));

    return self;
}

static VALUE
rg_remove_all_tags(int argc, VALUE *argv, VALUE self)
{
    VALUE start, end;
    GtkTextIter start_iter, end_iter;

    rb_scan_args(argc, argv, "02", &start, &end);

    gtk_text_buffer_remove_all_tags(_SELF(self),
                                    RVAL2STARTITER(self, start, start_iter),
                                    RVAL2ENDITER(self, end, end_iter));

    return self;
}

static VALUE
rg_bounds(VALUE self)
{
    GtkTextIter start, end;

    gtk_text_buffer_get_bounds(_SELF(self), &start, &end);

    return rb_ary_new3(2, GTKTEXTITER2RVAL(&start), GTKTEXTITER2RVAL(&end));
}

void 
Init_gtk_textbuffer(VALUE mGtk)
{
    rb_mGtk = mGtk;
    VALUE RG_TARGET_NAMESPACE = G_DEF_CLASS(GTK_TYPE_TEXT_BUFFER, "TextBuffer", mGtk);

    RG_DEF_METHOD(initialize, -1);
    RG_DEF_METHOD(line_count, 0);
    RG_DEF_METHOD(char_count, 0);

    G_REPLACE_SET_PROPERTY(RG_TARGET_NAMESPACE, "text", txt_set_text, 1);
    RG_DEF_METHOD(insert, -1);
    RG_DEF_METHOD(backspace, 3);
    RG_DEF_METHOD(insert_at_cursor, 1);
    RG_DEF_METHOD(insert_interactive, 3);
    RG_DEF_METHOD(insert_interactive_at_cursor, 2);
    RG_DEF_METHOD(insert_range, 3);
    RG_DEF_METHOD(insert_range_interactive, 4);

    RG_DEF_METHOD(delete, -1);
    RG_DEF_METHOD(delete_interactive, -1);

    RG_DEF_METHOD(get_text, -1);
    G_REPLACE_GET_PROPERTY(RG_TARGET_NAMESPACE, "text", txt_get_text_all, 0);
    RG_DEF_METHOD(get_slice, -1);
    RG_DEF_METHOD(slice, 0);

    RG_DEF_METHOD(create_child_anchor, 1);

    RG_DEF_METHOD(create_mark, 3);
    RG_DEF_METHOD(add_mark, 2);
    RG_DEF_METHOD(delete_mark, 1);

    RG_DEF_METHOD(get_mark, 1);
/* Comment out because this method's name is very bad.
   Use Gtk::TextBuffer#get_mark("insert") instead.
    RG_DEF_METHOD(get_insert, 0);
*/
    RG_DEF_METHOD(selection_bound, 0);
    RG_DEF_METHOD(place_cursor, 1);
    RG_DEF_METHOD(select_range, 2);
    RG_DEF_METHOD_P(modified, 0);
    RG_DEF_METHOD(set_modified, 1);

    RG_DEF_METHOD(add_selection_clipboard, 1);
    RG_DEF_METHOD(remove_selection_clipboard, 1);
    RG_DEF_METHOD(deserialize, 4);
    RG_DEF_METHOD_P(deserialize_can_create_tags, 1);
    RG_DEF_METHOD(deserialize_set_can_create_tags, 2);
    RG_DEF_METHOD(deserialize_formats, 0);
    RG_DEF_METHOD(serialize_formats, 0);
    RG_DEF_METHOD(register_deserialize_format, 1);
    RG_DEF_METHOD(register_deserialize_target, 1);
    RG_DEF_METHOD(register_serialize_format, 1);
    RG_DEF_METHOD(register_serialize_target, 1);
    RG_DEF_METHOD(serialize, 4);
    RG_DEF_METHOD(unregister_deserialize_format, 1);
    RG_DEF_METHOD(unregister_serialize_format, 1);
    RG_DEF_METHOD(cut_clipboard, 2);
    RG_DEF_METHOD(copy_clipboard, 1);
    RG_DEF_METHOD(paste_clipboard, 3);

    RG_DEF_METHOD(selection_bounds, 0);
    RG_DEF_METHOD(delete_selection, -1);

    RG_DEF_METHOD(begin_user_action, 0);
    RG_DEF_METHOD(end_user_action, 0);

    RG_DEF_METHOD(start_iter, 0);
    RG_DEF_METHOD(end_iter, 0);
    RG_DEF_METHOD(get_iter_at, 1);
    RG_DEF_METHOD(bounds, 0);
    RG_DEF_METHOD(move_mark, 2);

    RG_DEF_METHOD(create_tag, 2);
    RG_DEF_METHOD(apply_tag, -1);
    RG_DEF_METHOD(remove_tag, -1);
    RG_DEF_METHOD(remove_all_tags, -1);
}
