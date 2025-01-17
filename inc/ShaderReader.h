char* textFileRead(const char* fn) {


	FILE* fp;
	char* content = NULL;

	int count = 0;

	if (fn != NULL) {

		if (fopen_s(&fp, fn, "rt") == 0) {

			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);
			if (count > 0) {
				content = (char*)malloc(sizeof(char) * (count + 1));
				count = fread_s(content, sizeof(char) * (count + 1), sizeof(char), count, fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}

int textFileWrite(const char* fn, const char* s) {

	FILE* fp;
	int status = 0;

	if (fn != NULL) {
		if (fopen_s(&fp, fn, "r+t") == 0) {
			if (fwrite(s, sizeof(char), strlen(s), fp) == strlen(s))
				status = 1;
			fclose(fp);
		}
	}
	return(status);
}