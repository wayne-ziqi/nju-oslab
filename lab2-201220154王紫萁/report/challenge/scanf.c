boolean tokenize(char *in, int *left, int *right, char split);

void str2dec(char *in, int start, int right, int *target);

void str2hex(char *in, int start, int right, uint32_t *target);

void str2str(char *in, int start, int right, char *target);

void delSpace(const char *string, int *start);

void scanf(const char* format, ...){
	int i = 0; // format index
    int count = 0; // buffer index
	char buffer[MAX_BUFFER_SIZE];
//    int index=0; // parameter index
    m_va_list ap;// address of format in stack
    int state = 0; // 0: legal character; 1: '%'; 2: illegal format
    int *addr_decimal = 0;
    uint32_t *addr_hexadecimal = 0;
    char *addr_string = 0;
    char *addr_character = 0;
	getStr(buffer, MAX_BUFFER_SIZE);
    // printf("%s\n", buffer);
//    void* para=0;
    m_va_start(ap, format);
	while(format[i]!=0){
		switch (state) {
            case 0:
                if (format[i] != '%') {
                    i++;
                } else {
                    state = 1;
                    i++;
                }
                break;
            case 1:
                switch (format[i]) {
					
                    case 'd': {//find split
                        int last = i + 1;
                        delSpace(format, &last);
                        char split = format[last];
                        delSpace(buffer, &count);
                        //find right boundary
                        int right = count;
                        boolean tokenyes = tokenize(buffer, &count, &right, split);
                        if (!tokenyes) {
                            state = 2;
                            break;
                        }
                        addr_decimal = m_va_arg(ap, int*);
                        str2dec(buffer, count, right, addr_decimal);
                        count = right + 1;
                        state = 0;
                        i = last + 1;
                        break;
                    }
                    case 'x':{
                        //find split
                        int last = i + 1;
                        delSpace(format, &last);
                        char split = format[last];
                        delSpace(buffer, &count);
                        //find right boundary
                        int right = count;
                        boolean tokenyes = tokenize(buffer, &count, &right, split);
                        if (!tokenyes) {
                            state = 2;
                            break;
                        }
                       addr_hexadecimal = m_va_arg(ap, uint32_t*);
                        str2hex(buffer, count, right, addr_hexadecimal);
                        count = right + 1;
                        state = 0;
                        i = last + 1;
                        break;
                    }
                    case 's':{
                        delSpace(buffer, &count);
                        int right = count;

                        boolean tokenyes = tokenize(buffer, &count, &right, ' ');
						if (!tokenyes) {
                            state = 2;
                            break;
                        }
                        addr_string = m_va_arg(ap, char*);
                        str2str(buffer, count, right, addr_string);
                        count = right + 1;
                        state = 0;
                        i++;
                        break;
                    }
                    case 'c':
                        delSpace(buffer, &count);
                        addr_character = m_va_arg(ap, char*);
                        *addr_character = buffer[count];
                        state = 0;
                        i++;
                        break;
                    default:
                        state = 2;
                        break;
                }
                break;
            case 2:
                state = 0;
                i++;
                break;
        }
	}
	m_va_end(ap);
}

void delSpace(const char *string, int *start) {
    while (string[*start] != 0 && string[*start] == ' ') {
        (*start)++;
    }
//    (*start)--;
}
boolean tokenize(char *in, int *left, int *right, char split) {
    while (in[*right] != 0 && in[*right] != split)(*right)++;
//    (*right)--;
    if (in[*right] == split)return TRUE;
    else return FALSE;
}
void str2dec(char *in, int start, int right, int *target) {
    int result = 0;
    for (int i = start; i < right; i++) {
        result = result * 10 + in[i] - '0';
    }
    *target = result;
}
void str2hex(char *in, int start, int right, uint32_t *target) {
    int result = 0;
    for (int i = start; i < right; i++) {
        int now = 0;
        if (in[i] >= '0' && in[i] <= '9') now = in[i] - '0';
        else if (in[i] >= 'a' && in[i] <= 'f') now = in[i] - 'a' + 10;
        result = result * 16 + now;
    }
    *target = result;
}

void str2str(char *in, int start, int right, char *target) {
    int cnt = 0;
    for (int i = start; i < right; i++) {
        target[cnt++] = in[i];
    }
    target[cnt++] = 0;
}

