#include "mposditem.hpp"

extern "C" {
#define new __new
#include <sub/sub.h>
#undef new
}

struct OsdData {
	int x = 0, y = 0, w = 0, h = 0, stride = 0;
	QByteArray data;
	quint32 lut[256];
	sub_bitmap_format format = SUBBITMAP_EMPTY;
};

struct MpOsdItem::Data {
	int loc_tex_data = 0, loc_tex_lut = 0;
	bool redraw = false;
	QRectF vertextRect = {.0, .0, .0, .0}, textureRect = {0.0, 0.0, 1.0, 1.0};
	QRectF osdRect = {0, 0, 0, 0};
	OsdData osd;
	QMutex mutex;
	QSize frameSize = {1, 1};
	bool newFrame = false;
	bool reposition = false;
	bool formatChanged = true;
	int prevId = -1, prevPosId = -1;
	bool called = false;
	sub_bitmap_format shaderFormat = SUBBITMAP_INDEXED;
};

MpOsdItem::MpOsdItem(QQuickItem *parent)
: TextureRendererItem(2, parent), d(new Data) {
	setVisible(false);
}

MpOsdItem::~MpOsdItem() {
	delete d;
}

void MpOsdItem::setFrameSize(const QSize &size) {
	d->frameSize = size;
	update();
}

void MpOsdItem::customEvent(QEvent *event) {
	TextureRendererItem::customEvent(event);
	if (event->type() == ShowEvent) {
		setVisible(true);
		update();
	} else if (event->type() == HideEvent) {
		setVisible(false);
	}
}

void MpOsdItem::beginNewFrame() {
	d->newFrame = true;
	d->called = false;
}

void MpOsdItem::endNewFrame() {
	d->newFrame = false;
	if (!d->called)
		qApp->postEvent(this, new QEvent((QEvent::Type)(HideEvent)));
	d->called = false;
}

void MpOsdItem::link(QOpenGLShaderProgram *program) {
	TextureRendererItem::link(program);
	d->loc_tex_data = program->uniformLocation("tex_data");
	d->loc_tex_lut = program->uniformLocation("tex_lut");
}

void MpOsdItem::bind(const RenderState &state, QOpenGLShaderProgram *program) {
	TextureRendererItem::bind(state, program);
	program->setUniformValue(d->loc_tex_data, 0);
	program->setUniformValue(d->loc_tex_lut, 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture(0));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, texture(1));
	glActiveTexture(GL_TEXTURE0);
}

void MpOsdItem::updateTexturedPoint2D(TexturedPoint2D *tp) {
	const auto exp_x = width()/d->frameSize.width();
	const auto exp_y = height()/d->frameSize.height();
	const QPointF pos(d->osdRect.x()*exp_x, d->osdRect.y()*exp_y);
	const QSizeF size(d->osdRect.width()*exp_x, d->osdRect.height()*exp_y);
	set(tp, QRectF(pos, size), d->textureRect);
}

void MpOsdItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) {
	TextureRendererItem::geometryChanged(newGeometry, oldGeometry);
	setGeometryDirty();
	update();
}

void MpOsdItem::draw(sub_bitmaps *imgs) {
	d->called = true;
	if (imgs->num_parts > 0) {
		auto &img = imgs->parts[0];
		if (d->prevId == imgs->bitmap_id && d->prevPosId == imgs->bitmap_pos_id)
			return;
		d->mutex.lock();
		if (d->prevId != imgs->bitmap_id) {
			d->osd.format = imgs->format;
			d->osd.w = img.dw;
			d->osd.h = img.dh;
			d->osd.x = img.x;
			d->osd.y = img.y;
			if (imgs->format == SUBBITMAP_INDEXED) {
				d->osd.stride = d->osd.w%4 ? ((d->osd.w/4)+1)*4 : d->osd.w;
				d->osd.data.resize(d->osd.stride*d->osd.h);
				auto bitmap = reinterpret_cast<osd_bmp_indexed*>(img.bitmap);
				auto from = bitmap->bitmap;
				auto to = d->osd.data.data();
				for (int i = 0; i < d->osd.h; ++i) {
					memcpy(to, from, d->osd.w);
					to += d->osd.stride;
					from += img.stride;
				}
				memcpy(d->osd.lut, bitmap->palette, 256*4);
				d->textureRect.setWidth((double)d->osd.w/(double)d->osd.stride);
			} else {
				d->osd.stride = img.stride;
				d->osd.data.resize(d->osd.stride*d->osd.h);
				memcpy(d->osd.data.data(), img.bitmap, d->osd.data.size());
				d->textureRect.setWidth((double)(d->osd.w*4-1)/(double)d->osd.stride);
			}
			d->prevId = imgs->bitmap_id;
			d->redraw = true;
		}
		if (d->prevPosId != imgs->bitmap_pos_id) {
			d->osd.x = img.x;
			d->osd.y = img.y;
			d->reposition = true;
		}
		d->mutex.unlock();
		qApp->postEvent(this, new QEvent((QEvent::Type)(ShowEvent)));
	}
}


void MpOsdItem::beforeUpdate() {
	if (d->redraw) {
		if (_Change(d->shaderFormat, d->osd.format)) {
			resetNode();
		} else
			initializeTextures();
		d->redraw = false;
	}
	if (d->reposition) {
		setGeometryDirty();
		d->reposition = false;
	}
}

void MpOsdItem::initializeTextures() {
	d->mutex.lock();
	if (d->shaderFormat == SUBBITMAP_INDEXED) {
		glBindTexture(GL_TEXTURE_2D, texture(0));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, d->osd.stride, d->osd.h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, d->osd.data.data());
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_1D, texture(1));
		glTexImage1D(GL_TEXTURE_1D, 0, 4, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, d->osd.lut);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	} else if (d->shaderFormat == SUBBITMAP_RGBA) {
		glBindTexture(GL_TEXTURE_2D, texture(0));
		glTexImage2D(GL_TEXTURE_2D, 0, 4, d->osd.stride/4, d->osd.h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, d->osd.data.data());
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	d->osdRect = QRectF(d->osd.x, d->osd.y, d->osd.w, d->osd.h);
	d->mutex.unlock();
	setGeometryDirty();
}

const char *MpOsdItem::fragmentShader() const {
	const char *shader = nullptr;
	if (d->shaderFormat == SUBBITMAP_INDEXED) {
		shader = (R"(
			uniform sampler2D tex_data;
			uniform sampler1D tex_lut;
			varying highp vec2 qt_TexCoord;
			void main() {
				float c = texture2D(tex_data, qt_TexCoord).x;
				gl_FragColor = texture1D(tex_lut, c);
			}
		)");
	} else {
		shader = (R"(
			uniform sampler2D tex_data;
			varying highp vec2 qt_TexCoord;
			void main() {
				gl_FragColor = texture2D(tex_data, qt_TexCoord);
			}
		)");
	}
	return shader;
}