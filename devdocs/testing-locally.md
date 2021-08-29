Testing Locally
==
klaculate is developed on a [FreeBSD](https://www.FreeBSD.org/) system with [`www/nginx`](https://freshports.org/www/nginx/) installed.
Since this is a web app, it follows that you need a web server and a web browser installed.  Any browser should do fine.

I'm assuming you already know how to install stuff on FreeBSD and get Xorg with a pretty desktop going, so I'm going to spare all those details.

Here's what you need in your `/usr/local/etc/nginx/nginx.conf` to play with klaculate locally:
```nginx
    server {
		listen 80;
		server_name .klac.localhost;
		root /full-path-to-klaculate/klaculate/web/;

		location / {
			add_header Cache-Control max-age=86400;
			add_header TryFile $uri;
			try_files $uri @pass;
		}

		location @pass {
			add_header Cache-Control "no-cache, no-store";
			include fastcgi_params;
			fastcgi_pass unix:/tmp/klaculate.sock;
		}
	}
```

Obviously, modify the `root` to point to the appropriate folder.

You also need a symlink in `/usr/home/` that points to where `klaculate` got checked out.  Here's mine:
```ls
# ls -l /usr/home/klaculate
lrwxr-xr-x   1 root        wheel        40 Aug 22 03:22 klaculate@ -> /usr/home/jlhawkwell/Projects/klaculate/
```
