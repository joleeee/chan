int cmpstr(const void* a, const void* b) {
	return strcmp(*(const char**)a, *(const char**)b);
}

int revcmpstr(const void* a, const void* b) {
	return strcmp(*(const char**)b, *(const char**)a);
}

int isalphanumerical(char *s){
	while(*s){
		if(!(*s >= 'A' && *s <= 'Z' || *s >= 'a' && *s <= 'z' || *s >= '0' && *s <= '9'))
			return 0;
		s++;
	}
	return 1;
}

// http://www.cse.yorku.ca/~oz/hash.html
const unsigned long hash(const unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

// https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/
char *escape(char *str, char *find, char *rep){
	char *result;
	int i, cnt=0;
	int findlen = strlen(find);
	int replen = strlen(rep);
	for(i=0; str[i] != '\0'; i++){
		if(strstr(&str[i], find) == &str[i]){
			cnt++;
			i += findlen-1;
		}
	}

	result = (char*)malloc(i + cnt*(replen-findlen) + 1);

	i=0;
	while(*str){
		if(strstr(str, find) == str){
			strcpy(&result[i], rep);
			i+=replen;
			str+=findlen;
		}
		else
			result[i++]=*str++;
	}
	result[i] = '\0';
	return result;
}

void urldecode(char *dst, const char *src)
{
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit(a) && isxdigit(b))) {
                        if (a >= 'a')
                                a -= 'a'-'A';
                        if (a >= 'A')
                                a -= ('A' - 10);
                        else
                                a -= '0';
                        if (b >= 'a')
                                b -= 'a'-'A';
                        if (b >= 'A')
                                b -= ('A' - 10);
                        else
                                b -= '0';
                        *dst++ = 16*a+b;
                        src+=3;
                } else if (*src == '+') {
                        *dst++ = ' ';
                        src++;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}

