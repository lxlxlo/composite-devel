
ICONS = \
  icons.png \
  icon_play.png \
  icon_stop.png \
  icon_matrix.png

all: $(ICONS)

clean:
	-rm -f $(ICONS)
	-rm *~

icons.png: icons.svg
	convert -background none $< $@

##
## ICONS CONFIG
##

ICON_SIZE=128

icon_%.png: icons.png
	## Might as well update them all
	K=0 ; \
	for N in play stop matrix ; do \
		convert $< -crop 128x128+$$(($$K*$(ICON_SIZE)))+0 icon_$${N}.png ; \
		K=$$(($$K+1)) ; \
	done
