#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <parser.h>
#include <parserInternals.h>

#define CHAR_BUFFER 1024

static void start_document(void *ctx) {
	printf("Document start\n");
}

static void end_document(void *ctx) {
	printf("Document end\n");
}

static void start_element(void *ctx, const CHAR *name, const CHAR **attrs) {
	const char *attr_ptr;
	int curr_attr = 0;

	printf("Element %s started\n", name);
	if(attrs) {
		attr_ptr = *attrs;
		while(attr_ptr) {
			printf("\tAttribute %s\n", attr_ptr);
			curr_attr++;
			attr_ptr = *(attrs + curr_attr);
		}
	}
}

static void end_element(void *ctx, const CHAR *name) {
	printf("Element %s ended\n", name);
}

static void chars_found(void *cts, const CHAR *chars, int len) {
	char buff[CHAR_BUFFER+1];

	if(len > CHAR_BUFFER) len = CHAR_BUFFER;

	strncpy(buff, chars, len);
	buff[len] = '\0';
	printf("Found %d characters: %s\n", len, buff);
}

static xmlSAXHandler mySAXParseCallbacks;

int main(void)
{
	xmlParserCtxtPtr ctxt_ptr;

	memset(&mySAXParseCallbacks, sizeof(mySAXParseCallbacks), 0);
	mySAXParseCallbacks.startDocument = start_document;
	mySAXParseCallbacks.endDocument = end_document;
	mySAXParseCallbacks.startElement = start_element;
	mySAXParseCallbacks.endElement = end_element;
	mySAXParseCallbacks.characters = chars_found;

	ctxt_ptr = xmlCreateFileParserCtxt("dvdcatalog.xml");
	if(!ctxt_ptr) {
		fprintf(stderr, "Failed to create file parser\n");
		exit(EXIT_FAILURE);
	}

	ctxt_ptr->sax = &mySAXParseCallbacks;

	xmlParseDocument(ctxt_ptr);
	if(!ctxt_ptr->wellFormed) {
		fprintf(stderr, "Document not well formed\n");
	}
	ctxt_ptr->sax = NULL;

	printf("XML version %s, encoding %s\n",
		ctxt_ptr->version, ctxt_ptr->encoding);

	xmlFreeParserCtxt(ctxt_ptr);

	printf("Parsing complete\n");
	exit(EXIT_SUCCESS);
}

