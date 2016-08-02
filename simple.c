#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>

#include <string.h>
#include <errno.h>

struct content {
	char   *data;
	size_t  len;
};
struct headers {
	char   **k;
	char   **v;
	size_t   len;
};

int init_content(struct content *c)
{
	c->len = 0;
	if ((c->data = malloc(c->len + 1)) == NULL) {
		fprintf(stderr, "malloc() failed\n");
		return 1;
	}
	c->data[0] = '\0';
	return 0;
}

int init_headers(struct headers *h)
{
	h->len = 0;
	if ((h->k = malloc(sizeof(char *) * 10)) == NULL) {
		return 1;
	}
	if ((h->v = malloc(sizeof(char *) * 10)) == NULL) {
		return 1;
	}
	return 0;
}

int add_header(struct headers *h, const char *k, const char *v)
{
	if (h->len % 10 == 7) {
		if ((h->k = realloc(h->k, sizeof(char *) * (h->len + 3 + 10))) == NULL) {

			return 1;
		}
		if ((h->v = realloc(h->v, sizeof(char *) * (h->len + 3 + 10))) == NULL) {

			return 1;
		}
	}
	h->k[h->len] = strdup(k);
	h->v[h->len] = strdup(v);
	h->len++;
	return 0;
}

static char * get_header(struct headers *h, const char *k)
{
	for (int i = 0; i < h->len; i++)
		if (strcmp(h->k[i], k) == 0)
			return h->v[i];
	return NULL;
}
static char ** get_idx(struct headers *h, int k)
{
	if (k <= h->len) {
		char **str = malloc(sizeof(char *));
		str[0] = h->k[k];
		str[1] = h->v[k];
		return str;
	}
	return NULL;
}
static size_t buffer_content(void *buf, size_t each, size_t n, struct content *c)
{
	size_t new_len = c->len + each * n;
	c->data = realloc(c->data, new_len + 1);
	if (c->data == NULL) {
		
		return 0;
	}
	memcpy(c->data + c->len, buf, each * n);
	c->data[new_len] = '\0';
	c->len = new_len;
	return each * n;
}

static size_t buffer_headers(char *buf, size_t each, size_t n, struct headers *h)
{
	if (strcmp(buf, "\r\n") == 0) return each * n;
	char *token = strdup(buf);
	if (strtok(token, ": ") != NULL) {
		if (strncmp(token, "HTTP", 4) == 0) return each * n;
		memmove(buf, buf + strlen(token) + 2, 3 + strlen(buf + strlen(token)));
		buf[strlen(buf) - 1] = '\0';
		add_header(h, token, buf);
	}
	return each * n;
}

int main(int argc, char *argv[])
{
	char *address = "https://danmolik.com";
	if (argc > 1) {
		//free(address);
		address = strdup(argv[1]);
	}
	char *agent  = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2623.112 Safari/537.36";
	if (argc > 2) {
		agent = strdup(argv[2]);
	}
	int res;
	CURL *ua;
	if ((res = curl_global_init(CURL_GLOBAL_SSL)) != 0) {
		fprintf(stderr, "failed to globalize curl, %s\n", curl_easy_strerror(res));
		return 1;
	}
	if ((ua = curl_easy_init()) == NULL) {
		fprintf(stderr, "failed to initialize curl, %s\n", strerror(errno));
		return 1;
	}

	struct content c;
	init_content(&c);
	struct headers h;
	init_headers(&h);

	curl_easy_setopt(ua, CURLOPT_USERAGENT, agent);
	curl_easy_setopt(ua, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
	curl_easy_setopt(ua, CURLOPT_HEADERFUNCTION, buffer_headers);
	curl_easy_setopt(ua, CURLOPT_HEADERDATA, &h);
	curl_easy_setopt(ua, CURLOPT_WRITEFUNCTION, buffer_content);
	curl_easy_setopt(ua, CURLOPT_WRITEDATA, &c);
	curl_easy_setopt(ua, CURLOPT_URL, address);
	if ((res = curl_easy_perform(ua)) != 0) {
		fprintf(stderr, "failed to get %s, %s", address, curl_easy_strerror(res));
		return 1;
	}

	for (int i=0; i < h.len; i++) {
		char **header = get_idx(&h, i);
		// printf("%s :: %s\n", header[0], header[1]);
	}

	curl_easy_cleanup(ua);
	return 0;
}
