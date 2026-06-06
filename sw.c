#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include <dbus/dbus.h>

static struct winsize ws;

enum Level
{
	LOW,
	NORMAL,
	CRITICAL,
};

typedef enum Level Level;

typedef struct Notification Notification;

struct Notification
{
	char* application;
	unsigned id;
	char* icon;
	char* summary;
	char* body;
	char* urgency;
	Level level;
	int timeout;
};

void
send_notification(Notification* n)
{
	DBusConnection* connection;
	DBusMessage* message;

	connection = dbus_bus_get(DBUS_BUS_SESSION, 0);
	if(!connection){
		fprintf(stderr, "Failed to create D-Bus message\n");
		return;
	}
	message = dbus_message_new_method_call(
		"org.freedesktop.Notifications",
		"/org/freedesktop/Notifications",
		"org.freedesktop.Notifications",
		"Notify"
	);

	DBusMessageIter iter[4];
	dbus_message_iter_init_append(message, iter);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &n->application);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, &n->id);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &n->icon);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &n->summary);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &n->body);
	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "s", iter + 1);
	dbus_message_iter_close_container(iter, iter + 1);
	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{sv}", iter + 1);
	dbus_message_iter_open_container(iter + 1, DBUS_TYPE_DICT_ENTRY, 0, iter + 2);

	dbus_message_iter_append_basic(iter + 2, DBUS_TYPE_STRING, &n->urgency);
	dbus_message_iter_open_container(iter + 2, DBUS_TYPE_VARIANT, "y", iter + 3);

	dbus_message_iter_append_basic(iter + 3, DBUS_TYPE_BYTE, &n->level);
	dbus_message_iter_close_container(iter + 2, iter + 3);
	dbus_message_iter_close_container(iter + 1, iter + 2);
	dbus_message_iter_close_container(iter, iter + 1);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_INT32, &n->timeout);

	dbus_connection_send(connection, message, 0);
	dbus_connection_flush(connection);

	dbus_message_unref(message);
	dbus_connection_unref(connection);
}

void
print_time(int t)
{
	int h, m, s, i;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

	for (i = 0; i < ws.ws_col/2; i++)
		printf("%*s\n", ws.ws_col, " ");

	h = t / 3600;
	m = (t % 3600) / 60;
	s = t % 60;
	printf("\r\033[K%*s%02d:%02d:%02d\n", (ws.ws_col/2)- 3, " ", h, m, s);

	for(i = (ws.ws_row/2); i > 1; i--)
		printf("\r\033[K\n");

	printf("\033]0;sw - %02d:%02d:%02d\007", h, m, s);
	printf("\033[%dA", (int)(ws.ws_row));
	fflush(stdout);
	sleep(1);
}

void
quit(const char* t)
{
	int len, i;

	for (i = 0; i < ws.ws_col/2; i++)
		printf("%*s\n", ws.ws_col, " ");

	len = strlen(t)/2;
	printf("\r%*s%s%*s\n\a", (ws.ws_col/2)-len+1, " ", t,(ws.ws_row/2)-len, " ");

	for (i = 0; i < (ws.ws_row/2)-1; i++)
		printf("%*s\n", ws.ws_col, " ");

	printf("\033]0;Terminal\007");
	printf("\033[?25h");
}

void
handle_sigint(int sig) {
	(void)sig;
	quit("ABORT!!!");
	exit(0);
}

int
main(int argc, char* argv[])
{
	int t;
	char msg[1024];

	signal(SIGINT, handle_sigint);

	memset(msg, '\0', sizeof(msg));

	if(!isatty(fileno(stdout))){
		fprintf(stderr, "Please run in real Terminal\n");
		return 1;
	}
	if(argc > 1){
		int h, m, s;
		const char* p;

		h = 0;
		m = 0;
		s = 0;
		for(p = argv[1]; *p; p++){
			char *end;
			long value;

			end = "";
			if (p == end) {
				fprintf(stderr, "Expected a number at: '%s'\n", p);
				return 2;
			}

			value = strtol(p, &end, 10);
			p = end;

			switch(*p){
			case 'h':
				h = (int)value;
				break;
			case 'm':
				m = (int)value;
				break;
			case 's':
				s = (int)value;
				break;
			default:
				fprintf(stderr, "Unknown unit '%c' at: '%s'\n", *p, p);
				fprintf(stderr, "Falid unit is:\n");
				fprintf(stderr, "       - h for hour\n");
				fprintf(stderr, "       - m for minutes\n");
				fprintf(stderr, "       - s for second\n");
				fprintf(stderr, " example: 1m30s for 1 minutes 30 second\n");
				return 3;
			}
		}
		t = (h*3600) + (m*60) + (s);
		if(t == 0){
			fprintf(stderr, "Please provide correct value\n");
			return 4;
		}
		if(argc > 2){
			int i;

			for(i = 2; i < argc; i++){
				strcat(msg, argv[i]);
				if(i != argc-1)
					strcat(msg, (char*)" ");
			}
		}
	}else
		t = 0;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	printf("\033]0;sw - \007");
	printf("\033[?25l");

	if(t != 0)
		for (; t >= 0; t--)
			print_time(t);
	else
		for(;;)
			print_time(t++);

	if(msg[0] == '\0')
		strcpy(msg, "TIME OUT!!");

	printf("%s\n", msg);

	send_notification(&(Notification){
			.application = "sw",
			.id          = 0,
			.icon        = "dialog-information",
			.summary     = "sw",
			.body        = msg,
			.urgency     = "urgency",
			.level       = NORMAL,
			.timeout     = 5 * 1000,
		});

	quit(msg);

	return 0;
}
