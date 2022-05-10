#!/bin/sh
rsync --delete --progress -hr web/ rk:/usr/home/klaculate/web/
rsync --delete --progress -hr templates/ rk:/usr/home/klaculate/templates/
if [ -x build/bin/klaculate ]; then
	ssh rk doas service klac stop
	rsync --delete --progress -hr build/bin/ rk:/usr/home/klaculate/bin/
	ssh rk doas service klac start
fi

