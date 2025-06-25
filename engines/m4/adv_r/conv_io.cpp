/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/adv_r/conv_io.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/conv.h"
#include "m4/adv_r/chunk_ops.h"
#include "m4/adv_r/db_env.h"
#include "m4/core/errors.h"
#include "m4/vars.h"
#include "m4/m4.h"

namespace M4 {

#define NAME_SIZE (g_engine->getGameType() == GType_Riddle ? 12 : 8)

#define HIDDEN		    0x00000004
#define DESTROYED       0x00000008

#define INITIAL         1
#define PERSISTENT      2

#define CONV_OK          0
#define CONV_QUIT       -1
#define CONV_NEW        -2
#define CONV_BAIL       -3

#define CONV_UNKNOWN_MODE   0
#define CONV_GET_TEXT_MODE  1
#define CONV_SET_TEXT_MODE  2
#define CONV_GET_MESG_MODE  3
#define CONV_UPDATE_MODE    4

#define DECL_POINTER	1

void Converstation_Globals::syncGame(Common::Serializer &s) {
	if (s.isLoading())
		conv_reset_all();

	// Handle size
	uint32 count = convSave.size();
	s.syncAsUint32LE(count);
	if (s.isLoading())
		convSave.resize(count);

	// Sync buffer contents
	if (count)
		s.syncBytes(&convSave[0], count);
}

void Converstation_Globals::conv_reset_all() {
	convSave.clear();
}

/*------------------------------------------------------------------------*/

void cdd_init() {
	for (int i = 0; i < 16; i++) {
		_G(cdd).text[i] = nullptr;
		_G(cdd).snd_files[i] = nullptr;
	}

	_G(cdd).num_txt_ents = 0;
	Common::strcpy_s(_G(cdd).mesg, "");
	_G(cdd).mesg_snd_file = nullptr;
}

Conv *conv_get_handle(void) {
	return _GC(globConv);
}

void conv_set_handle(Conv *c) {
	_GC(globConv) = c;
}

void conv_resume(Conv *c) {
	conv_go(c);
}

void conv_resume() {
	conv_resume(conv_get_handle());
}

int conv_is_event_ready() {
	return _GC(event_ready);
}

void conv_set_event(int e) {
	_GC(event) = e;
	_GC(event_ready) = 1;
}

int conv_get_event() {
	_GC(event_ready) = 0;
	return _GC(event);
}

void conv_play(Conv *c) {
	conv_go(c);
}

void conv_play() {
	conv_play(conv_get_handle());
}

int32 conv_current_node() {
	if (conv_get_handle())
		return conv_get_handle()->node_hash;
	return 0;
}

int32 conv_current_entry() {
	return _GC(ent) - 1;
}

void conv_reset(const char *filename) {
	_GC(restore_conv) = 0;

	Conv *c = conv_load(filename, 1, 1, -1, false);
	conv_unload(c);
}


void conv_reset_all() {
	_G(conversations).conv_reset_all();
}

const char *conv_sound_to_play() {
	return _G(cdd).mesg_snd_file;
}

int32 conv_whos_talking() {
	return _G(cdd).player_non_player;
}

int ok_status(entry_chunk *entry) {
	if (entry->status & DESTROYED)
		return 0;

	if (entry->status & HIDDEN)
		return 0;

	return 1;
}

int conv_toggle_flags(entry_chunk *entry) {
	if (ok_status(entry))
		return (entry->status & 0x0000000e); //mask off INITIAL bit.
	return entry->status;
}

int32 conv_get_decl_val(Conv *c, decl_chunk *decl) {
	if (decl->flags == DECL_POINTER)
		return *c->_pointers[decl->addrIndex];

	return decl->val;
}

void conv_set_decl_val(Conv *c, decl_chunk *decl, int32 val) {
	if (decl->flags == DECL_POINTER) {
		decl->val = val;
		*c->_pointers[decl->addrIndex] = val;
	} else {
		decl->val = val;
	}
}

void conv_export_value(Conv *c, int32 val, int index) {
	int32 tag = 0, next;
	int i = 0;

	if (!c)
		return;

	const int32 ent_old = c->myCNode;
	int32 ent = 0;
	c->myCNode = 0;

	while (ent < c->chunkSize) {
		conv_ops_get_entry(ent, &next, &tag, c);

		if (tag == DECL_CHUNK) {
			if (i == index) {
				decl_chunk *decl = get_decl(c, ent);
				conv_set_decl_val(c, decl, val);
			}
			i++;
		}
		ent = next;
	}
	c->myCNode = ent_old;
}

void conv_export_value_curr(int32 val, int index) {
	conv_export_value(conv_get_handle(), val, index);
}

void conv_export_pointer(Conv *c, int32 *val, int index) {
	int32 tag = 0, next;
	int	i = 0;

	if (!c)
		return;

	const int32 ent_old = c->myCNode;
	int32 ent = 0;
	c->myCNode = 0;

	while (ent < c->chunkSize) {
		conv_ops_get_entry(ent, &next, &tag, c);

		switch (tag) {
		case DECL_CHUNK:
			if (i == index) {
				decl_chunk *decl = get_decl(c, ent);

				c->_pointers.push_back(val);
				decl->addrIndex = c->_pointers.size() - 1;
				decl->flags = DECL_POINTER;
			}
			i++;
			break;

		default:
			break;
		}
		ent = next;
	}
	c->myCNode = ent_old;
}

void conv_export_pointer_curr(int32 *val, int index) {
	conv_export_pointer(conv_get_handle(), val, index);
}

void conv_init(Conv *c) {
	switch (c->exit_now) {
	case CONV_OK:
	case CONV_QUIT:
		break;

	case CONV_BAIL:
	case CONV_NEW:
		if (c->myCNode != CONV_QUIT) {
			c->exit_now = CONV_NEW; //conv hasn't been run before. only done here once.
			c->myCNode = 0;
		}
		break;
		
	default:
		break;
	}
}

static int32 find_state(const char *s, char *c, int file_size) {
	char name[13];
	int32 size = 0, offset = 0;

	while (offset < file_size) {
		Common::strcpy_s(name, &c[offset]);

		if (!scumm_stricmp(name, s)) {
			offset += NAME_SIZE * sizeof(char);
			goto handled;
		}

		offset += NAME_SIZE * sizeof(char);
		if (offset < file_size) {
			memcpy(&size, &c[offset], sizeof(int32));
		}

		offset += size + sizeof(int32);
	}

	offset = -1;

handled:
	return offset;
}

void find_and_set_conv_name(Conv *c) {
	int32 ent = 0, tag = 0, next = 0;

	c->myCNode = 0;

	while (ent < c->chunkSize) {
		conv_ops_get_entry(ent, &next, &tag, c);

		if (tag == CONV_CHUNK) {
			conv_chunk *conv = get_conv(c, ent);
			assert(conv);
			Common::strcpy_s(_GC(conv_name), get_string(c, c->myCNode + ent + sizeof(conv_chunk)));
		}
		ent = next;
	}
}

static void conv_save_state(Conv *c) {
	//-------------------------------------------------------------------------------
	// Calculate amt_to_write by counting up the size of DECL_CHUNKs.
	// the number of ENTRY_CHUNKs affects the amt_to_write
	// also extract fname from the CONV_CHUNK

	int32 amt_to_write = 3 * sizeof(int32);	// Header size
	int32 ent = 0;
	int32 next, tag;	// receive conv_ops_get_entry results
	const int32 myCNode = c->myCNode;
	char fname[13];
	memset(fname, 0, 13);

	int32 num_decls = 0;
	int32 num_entries = 0;

	c->myCNode = 0;

	while (ent < c->chunkSize) {
		conv_chunk *conv;
		conv_ops_get_entry(ent, &next, &tag, c);

		switch (tag) {
		case CONV_CHUNK:
			conv = get_conv(c, ent);
			assert(conv);
			Common::strcpy_s(fname, get_string(c, c->myCNode + ent + sizeof(conv_chunk)));
			break;

		case DECL_CHUNK:
			num_decls++;
			amt_to_write += sizeof(int32);
			break;

		case ENTRY_CHUNK:
			num_entries++;
			break;

		default:
			break;
		}
		ent = next;
	}

	amt_to_write += (num_entries / 8) * sizeof(int32);
	if ((num_entries % 8) != 0)
		amt_to_write += sizeof(int32);	// Pad the sucker

	//-------------------------------------------------------------------------------
	// if consave data exists, read it in

	int32 file_size = 0;
	int32 offset;
	char *conv_save_buff;
	bool overwrite_file = false;

	if (!_GC(convSave).empty()) {
		file_size = _GC(convSave).size();

		conv_save_buff = (char *)mem_alloc(file_size, "conv save buff");
		if (!conv_save_buff)
			error_show(FL, 'OOM!');

		Common::copy(&_GC(convSave)[0], &_GC(convSave)[0] + file_size, &conv_save_buff[0]);

		//----------------------------------------------------------------------------
		// If this conversation already in conv data, overwrite it,
		// otherwise chuck out the buffer, and create a new buffer which is just
		// big enough to hold the new save data.

		offset = find_state(fname, conv_save_buff, file_size);

		if (offset != -1) {
			overwrite_file = true;
			/* int32 prev_size = */ READ_LE_UINT32(&conv_save_buff[offset]);
			/* prev_size += NAME_SIZE + sizeof(int32);*/
			offset += sizeof(int32);	// Skip header. (name + size)
		} else {
			// Append
			offset = 0;

			mem_free(conv_save_buff);
			conv_save_buff = (char *)mem_alloc(amt_to_write + NAME_SIZE + sizeof(int32), "conv save buff");
			if (!conv_save_buff)
				error_show(FL, 'OOM!');

			memcpy(&conv_save_buff[offset], fname, NAME_SIZE * sizeof(char));
			offset += NAME_SIZE * sizeof(char);
			WRITE_LE_UINT32(&conv_save_buff[offset], amt_to_write);
			offset += sizeof(int32);
		}
	} else {
		//----------------------------------------------------------------------------
		// Conv save dat didn't exist, so we set things up for a create here.

		offset = 0;

		conv_save_buff = (char *)mem_alloc(amt_to_write + NAME_SIZE + sizeof(int32), "conv save buff");
		if (!conv_save_buff)
			error_show(FL, 'OOM!');

		memcpy(&conv_save_buff[offset], fname, NAME_SIZE * sizeof(char));
		offset += NAME_SIZE * sizeof(char);
		WRITE_LE_UINT32(&conv_save_buff[offset], amt_to_write);
		offset += sizeof(int32);
	}

	//----------------------------------------------------------------------------
	// finish filling in conv_save_buff data with num of entries etc.

	WRITE_LE_INT32(&conv_save_buff[offset], myCNode);
	offset += sizeof(int32);

	WRITE_LE_UINT32(&conv_save_buff[offset], num_decls);
	offset += sizeof(int32);

	WRITE_LE_UINT32(&conv_save_buff[offset], num_entries);
	offset += sizeof(int32);

	int32 size = 3 * sizeof(int32);

	// fill in all the entries themselves

	int32 e_flags = 0;
	short flag_index = 0;

	ent = 0;
	c->myCNode = 0;

	int32 val;
	entry_chunk *entry;

	while (ent < c->chunkSize) {
		conv_ops_get_entry(ent, &next, &tag, c);
		decl_chunk *decl;

		switch (tag) {
		case DECL_CHUNK:
			decl = get_decl(c, ent);
			val = conv_get_decl_val(c, decl);

			WRITE_LE_UINT32(&conv_save_buff[offset], val);
			offset += sizeof(int32);
			size += sizeof(int32);
			break;

		case LNODE_CHUNK:
		case NODE_CHUNK:
			break;

		case ENTRY_CHUNK:
			entry = get_entry(c, ent);

			if (flag_index == 32) {
				flag_index = 0;

				WRITE_LE_UINT32(&conv_save_buff[offset], e_flags);
				offset += sizeof(int32);
				size += sizeof(int32);

				e_flags = 0;
			}

			e_flags |= ((entry->status & 0x0000000f) << flag_index);

			flag_index += 4;
			break;

		default:
			break;
		}

		ent = next;
	}

	// Copy the flags
	if (flag_index != 0) {
		WRITE_LE_UINT32(&conv_save_buff[offset], e_flags);
		// offset += sizeof(int32);
		size += sizeof(int32);
	}

	if (amt_to_write != size)
		error_show(FL, 'CNVS', "save_state: error! size written != size (%d %d)", amt_to_write, size);

	// Finally, write out the conversation data
	if (overwrite_file == true) {
		_GC(convSave).resize(file_size);
		Common::copy(conv_save_buff, conv_save_buff + file_size, &_GC(convSave)[0]);

	} else {
		// Append conversation
		const size_t oldSize = _GC(convSave).size();
		file_size = amt_to_write + NAME_SIZE + sizeof(int32);

		_GC(convSave).resize(_GC(convSave).size() + file_size);
		Common::copy(conv_save_buff, conv_save_buff + file_size, &_GC(convSave)[oldSize]);
	}

	mem_free(conv_save_buff);
}

static Conv *conv_restore_state(Conv *c) {
	int32 tag, next;

	short flag_index = 0;
	int32 val;
	int32 e_flags = 0;
	int32 myCNode;

	char fname[13];
	int file_size;

	int32 ent;
	c->myCNode = 0;

	find_and_set_conv_name(c);
	Common::strcpy_s(fname, _GC(conv_name));

	if (_GC(convSave).empty())
		file_size = -1;
	else
		file_size = _GC(convSave).size();

	if (file_size <= 0) {
		conv_init(c);
		return c;
	}

	char *conv_save_buff = (char *)mem_alloc(file_size, "conv save buff");
	if (!conv_save_buff)
		error_show(FL, 'OOM!');

	// ------------------

	Common::copy(&_GC(convSave)[0], &_GC(convSave)[0] + file_size, &conv_save_buff[0]);
	int32 offset = find_state(fname, conv_save_buff, file_size);

	if (offset == -1)
		goto i_am_so_done;

	// Skip header.
	offset += sizeof(int32);

	myCNode = READ_LE_INT32(&conv_save_buff[offset]);
	offset += sizeof(int32);

	/*int num_decls = */READ_LE_UINT32(&conv_save_buff[offset]);
	offset += sizeof(int32);

	/*int num_entries = */READ_LE_UINT32(&conv_save_buff[offset]);
	offset += sizeof(int32);

	ent = 0; c->myCNode = 0;

	while (ent < c->chunkSize) {
		conv_ops_get_entry(ent, &next, &tag, c);

		if (tag == DECL_CHUNK) {
			val = READ_LE_UINT32(&conv_save_buff[offset]);
			offset += sizeof(int32);
			decl_chunk *decl = get_decl(c, ent);

			conv_set_decl_val(c, decl, val);
		}

		ent = next;
	}

	ent = 0;
	c->myCNode = 0;

	while (ent < c->chunkSize) {
		conv_ops_get_entry(ent, &next, &tag, c);

		if (tag == ENTRY_CHUNK) {
			entry_chunk *entry = get_entry(c, ent);

			if (flag_index == 32) {
				flag_index = 0;
				//flag_num++;
			}

			if (flag_index == 0) {
				e_flags = READ_LE_UINT32(&conv_save_buff[offset]);
				offset += sizeof(int32);
			}

			val = (e_flags >> flag_index) & 0x0000000f;
			entry->status = val;

			flag_index += 4;
		}

		ent = next;
	}

	c->myCNode = myCNode;
	if (c->myCNode == CONV_QUIT) {
		c->exit_now = CONV_QUIT;

		conv_unload(c);
		c = nullptr;
	} else
		c->exit_now = CONV_OK;

i_am_so_done:

	mem_free(conv_save_buff);
	return c;
}

void conv_set_font_spacing(int32 h, int32 v) {
	_GC(conv_font_spacing_h) = h;
	_GC(conv_font_spacing_v) = v;
}


void conv_set_text_colours(int32 norm_colour, int32 norm_colour_alt1, int32 norm_colour_alt2,
	int32 hi_colour, int32 hi_colour_alt1, int32 hi_colour_alt2) {
	_GC(conv_normal_colour) = norm_colour;
	_GC(conv_normal_colour_alt1) = norm_colour_alt1;
	_GC(conv_normal_colour_alt2) = norm_colour_alt2;
	_GC(conv_hilite_colour) = hi_colour;
	_GC(conv_hilite_colour_alt1) = hi_colour_alt1;
	_GC(conv_hilite_colour_alt2) = hi_colour_alt2;
}

void conv_set_text_colour(int32 norm_colour, int32 hi_colour) {
	conv_set_text_colours(norm_colour, norm_colour, norm_colour, hi_colour, hi_colour, hi_colour);
}

void conv_set_default_hv(int32 h, int32 v) {
	_GC(conv_default_h) = h;
	_GC(conv_default_v) = v;
}

void conv_set_default_text_colour(int32 norm_colour, int32 hi_colour) {
	conv_set_text_colours(norm_colour, norm_colour, norm_colour, hi_colour, hi_colour, hi_colour);

	_GC(conv_default_normal_colour) = norm_colour;
	_GC(conv_default_hilite_colour) = hi_colour;
}

void conv_set_shading(int32 shade) {
	_GC(conv_shading) = shade;
}

void conv_set_box_xy(int32 x, int32 y) {
	_GC(glob_x) = x;
	_GC(glob_y) = y;
}

static void conv_set_disp_default(void) {
	_GC(conv_font_spacing_h) = _GC(conv_default_h);
	_GC(conv_font_spacing_v) = _GC(conv_default_v);
	_GC(conv_normal_colour) = _GC(conv_default_normal_colour);
	_GC(conv_hilite_colour) = _GC(conv_default_hilite_colour);
	_GC(conv_shading) = 75;
}

Conv *conv_load(const char *filename, int x1, int y1, int32 myTrigger, bool want_box) {
	char fullPathname[MAX_FILENAME_SIZE];

	term_message("conv_load");

	// Remember if player commands are on before we start the conversation
	_GC(playerCommAllowed) = _G(player).comm_allowed;
	_GC(interface_was_visible) = INTERFACE_VISIBLE;

	term_message("conv load:   %s", filename);

	if (want_box) {
		// If we want an interface box
		conv_set_disp_default();
		mouse_set_sprite(0);					// Also if we want a text box, lock the mouse into arrow mode
		mouse_lock_sprite(0);
		player_set_commands_allowed(false);		// with commands off

		// Hide the interface if it's visible
		if (INTERFACE_VISIBLE)
			interface_hide();
	}

	// if not in rooms.db, use actual filename
	char *str = env_find(filename);
	if (str)
		Common::strcpy_s(fullPathname, str);
	else
		Common::sprintf_s(fullPathname, "%s.chk", filename);

	SysFile fp(fullPathname);
	if (!fp.exists()) {
		// Force the file open
		error_show(FL, 'CNVL', "couldn't conv_load %s", fullPathname);
	}

	const int32 cSize = fp.size();

	if (conv_get_handle() != nullptr) {
		conv_unload();
	}

	Conv *convers = new Conv();
	convers->chunkSize = cSize;
	convers->conv = nullptr;
	convers->myCNode = 0;
	convers->exit_now = CONV_NEW;
	convers->node_hash = 0;
	convers->mode = CONV_GET_TEXT_MODE;
	convers->c_entry_num = 1;
	_GC(myFinalTrigger) = kernel_trigger_create(myTrigger);

	convers->conv = (char *)mem_alloc(cSize * sizeof(char), "conv char data");

	if (!fp.read((byte *)convers->conv, cSize)) {
		conv_set_handle(nullptr);
		delete convers;
		convers = nullptr;
		fp.close();

		return nullptr;
	}

	conv_swap_words(convers);
	find_and_set_conv_name(convers);

	_GC(glob_x) = x1;
	_GC(glob_y) = y1;

	if (want_box)
		set_dlg_rect();

	if (_GC(restore_conv))
		convers = conv_restore_state(convers);
	_GC(restore_conv) = 1;

	conv_set_handle(convers);

	fp.close();

	return convers;
}

void conv_load_and_prepare(const char *filename, int trigger, bool ignoreIt) {
	player_set_commands_allowed(false);

	if (!ignoreIt) {
		conv_load(filename, 10, 375, trigger, true);
		conv_set_shading(100);
		conv_set_text_colours(3, 1, 2, 22, 10, 14);
		conv_set_font_spacing(10, 2);
	}
}

void conv_unload(Conv *c) {
	mouse_unlock_sprite();

	if (_GC(interface_was_visible)) {	// Turn interface back on if it was on
		interface_show();
	}

	_GC(globConv) = nullptr;

	if (c)
		conv_save_state(c);

	player_set_commands_allowed(_GC(playerCommAllowed));

	_G(player).command_ready = false;
	_G(player).ready_to_walk = false;
	_G(player).need_to_walk = false;

	Common::strcpy_s(_G(player).verb, "");
	Common::strcpy_s(_G(player).noun, "");
	kernel_trigger_dispatchx(_GC(myFinalTrigger));

	if (c) {
		if (c->conv)
			mem_free(c->conv);
		delete c;
	}

	_GC(globConv) = c = nullptr;
}

void conv_unload() {
	conv_unload(conv_get_handle());
}

// only called if node is visible.
// gets the TEXT chunks inside a node.
int conv_get_text(int32 offset, int32 size, Conv *c) {
	int32 i = offset, tag, next;
	int	result = 0;

	size -= sizeof(entry_chunk);

	while (i < offset + size) {
		conv_ops_get_entry(i, &next, &tag, c);

		if (tag == TEXT_CHUNK) {
			result = 1;
			text_chunk *text = get_text(c, i);
			assert(text);
			const int32 text_len = conv_ops_text_strlen(get_string(c, c->myCNode + i + sizeof(text_chunk)));
			_G(cdd).snd_files[_G(cdd).num_txt_ents] = get_string(c, c->myCNode + i + sizeof(text_chunk));
			_G(cdd).text[_G(cdd).num_txt_ents] = get_string(c, c->myCNode + i + sizeof(text_chunk) + text_len);

			const int32 text_width = gr_font_string_width(_G(cdd).text[_G(cdd).num_txt_ents], 1);
			if (text_width > _GC(width))
				_GC(width) = text_width;

			_G(cdd).num_txt_ents++;
		}

		i = next;
	}
	return result;
}

} // End of namespace M4
