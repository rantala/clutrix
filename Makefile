clutrix: clutrix.c
	gcc -Wall -Wextra -g -O2 `pkg-config clutter-1.0 --cflags --libs` $< -o $@

clutrix-maemo: clutrix.c
	gcc -Wall -Wextra -g -O2 `pkg-config clutter-0.8 --cflags --libs` $< -o $@

clean:
	rm -f clutrix
