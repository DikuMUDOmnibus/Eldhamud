CC      = gcc
#PROF    = -p

#IMC2 - Comment out to disable IMC2 support
IMC = 1

W_FLAGS = -Wall -Wshadow -Wformat -Winline -Wpointer-arith -Wcast-align -Wredundant-decls -Wcast-qual -Wunused-macros
C_FLAGS = -g2 $(W_FLAGS) $(W_FLAGS2) $(PROF) -export-dynamic
L_FLAGS = -lm $(PROF) -lcrypt -ldl

C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c arena.c archery.c\
	  ban.c bank.c build.c \
	  clans.c color.c comm.c commands.c const.c creation.c \
	  db.c deity.c  \
	  fight.c \
	  handler.c hashstr.c hotboot.c house.c \
	  infochan.c interp.c \
	  liquids.c locker.c \
	  magic.c makeobjs.c mapper.c mental_skills.c misc.c mip.c mpxset.c mud_comm.c mud_prog.c \
	  overland.c \
	  pfiles.c player.c physical_skills.c \
	  questing.c \
	  random_armor.c random_desc.c random_weapons.c random_treasure.c remort.c reset.c \
	  save.c sha256.c shops.c skills.c special.c  \
	  tables.c tattoo.c train.c \
	  update.c \

O_FILES := $(patsubst %.c,ObjectFiles/%.o,$(C_FILES))

H_FILES = $(wildcard *.h) 

ifdef IMC
   C_FILES := imc.c $(C_FILES)
   O_FILES := ObjectFiles/imc.o $(O_FILES)
   C_FLAGS := $(C_FLAGS) -DIMC -DIMCSMAUG
endif

all:
	$(MAKE) -s eldhamud

eldhamud: $(O_FILES)
	rm -f eldhamud
	$(CC) -export-dynamic -o eldhamud $(O_FILES) $(L_FLAGS)
	echo "Done compiling mud.";
	chmod g+w eldhamud
	chmod a+x eldhamud
	chmod g+w $(O_FILES)

clean:
	rm -f *.o eldhamud *~ 
	rm -f ObjectFiles/*.o
	rm -f Headers/*~
	
ObjectFiles/%.o: %.c
	echo "  Compiling $@....";
	$(CC) -c $(C_FLAGS) $< -o $@

.c.o: mud.h
	$(CC) -c $(C_FLAGS) $<

indent:
	@indent -ts3 -nut -nsaf -nsai -nsaw -npcs -npsl -ncs -nbc -bls -prs -bap -cbi0 -cli3 -bli0 -l175 -lp -i3 -cdb -c1 -cd1 -sc -pmt $(C_FILES) 

