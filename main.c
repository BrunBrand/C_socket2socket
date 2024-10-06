/*
 *  socket2socket main program
 *
 *  How it works:
 *      The program aims to create a server on the local machine that forwards
 *      incoming messages to another server. Formally this can be used to establish
 *      connections between a machine that has no contact with the Internet, but connects
 *      to a proxy server that is connected to the Internet.
 *
 *  How to use:
 *      Give the address and port of the server to forward messages to, and
 *      the port to which the local server will listen for messages to be forwarded
 *
 *
 *
 *  TODO: Create GUI application with GTK
 *  WARNING: Testing in windows does not work now
*/

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <memory.h>
#include <gtk-4.0/gtk/gtk.h>
#include "socket2socket_lib/socket2socket_lib.h"

static GtkBuilder *builder = NULL;
static gboolean server_running = FALSE;

typedef struct {
    const char *remote_addr;
    const char *remote_port;
    const char *local_port;
} ServerConfig;

gpointer server_fn(gpointer data) {
    const ServerConfig conf = *(ServerConfig*)data;
    run_server(conf.remote_addr, conf.remote_port, conf.local_port);
    return NULL;
}

static void on_server_button_clicked(GtkWidget *widget, gpointer data) {
    GObject *textw1 = gtk_builder_get_object(builder, "textw1");
    GObject *textw2 = gtk_builder_get_object(builder, "textw2");
    GObject *textw3 = gtk_builder_get_object(builder, "textw3");

    ServerConfig conf = {
        .remote_addr = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(textw1))),
        .remote_port = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(textw2))),
        .local_port = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(textw3)))
    };

    static GThread *thread = NULL;

    if (server_running) {
        stop_server();
        if (thread != NULL) {
            g_thread_join(thread);
        }
        server_running = FALSE;
        gtk_button_set_label(GTK_BUTTON(widget), "Start Server");
    } else {
        thread = g_thread_new("server_thread", server_fn, &conf);
        gtk_button_set_label(GTK_BUTTON(widget), "Stop Server");
        server_running = TRUE;
    }
}

static void
activate (GtkApplication* app,
          gpointer        user_data)
{
    builder = gtk_builder_new_from_file("builder.ui");
    GObject *window = gtk_builder_get_object(builder, "window");
    GObject *button = gtk_builder_get_object(builder, "button1");

    // Configure window
    gtk_window_set_application(GTK_WINDOW(window), app);
    gtk_window_set_title (GTK_WINDOW (window), "Socket2Socket");
    gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);


    // Configure button signal
    g_signal_connect(button, "clicked", G_CALLBACK(on_server_button_clicked), NULL);

    gtk_window_present (GTK_WINDOW (window));

}


int main(const int argc, char **argv) {

#ifdef _WIN32
    WSADATA d;
    if(WSAStartup(MAKEWORD(2,2), &d)){
        exit_with_sys_msg("Failed to initialize");
    }
#endif


    if (argc < 4 || argc > 5) {
        printf("\nUsage: socket2socket remote_host remote_port local_port\n\n");
        exit(1);
    }

    printf("\nInitialised.\n");

    // const char *remote_host = argv[1];
    // const char *remote_port = argv[2];
    // const char *local_port = argv[3];


    GtkApplication *app = gtk_application_new("or.gtk.s2s", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    const int status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    return status;

    // run_server(remote_host, remote_port, local_port);

#ifdef _WIN32
    WSACleanup();
#endif

}
