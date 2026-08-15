"""Render the original DS3 keyboard/mouse prompt artwork.

The artwork geometry in this file was created from scratch for this package.
It does not read, trace, sample, or transform any third-party icon image.
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Callable

from PIL import Image, ImageDraw, ImageFilter, ImageFont


ROOT = Path(__file__).resolve().parent
MASTER_DIR = ROOT / "masters"
PNG_DIR = ROOT / "png"
PREVIEW_DIR = ROOT / "preview"
FONT_REGULAR = Path(r"C:\Windows\Fonts\georgia.ttf")
FONT_BOLD = Path(r"C:\Windows\Fonts\georgiab.ttf")

MASTER_SIZE = 512
FINAL_SIZE = 32
SCALE = MASTER_SIZE / FINAL_SIZE
FRAME_STYLE = "framed"

PARCHMENT = (242, 235, 213, 255)
PARCHMENT_DIM = (210, 199, 171, 255)
BRASS = (177, 137, 75, 255)
BRASS_LIGHT = (218, 178, 104, 255)
BRASS_DARK = (82, 60, 34, 255)
INK = (5, 5, 7, 255)


def u(value: float) -> int:
    """Scale a coordinate in final 32 px units to the master canvas."""

    return round(value * SCALE)


def points(values: list[tuple[float, float]]) -> list[tuple[int, int]]:
    return [(u(x), u(y)) for x, y in values]


def font(path: Path, final_px: float) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(str(path), u(final_px))


def plate_mask() -> Image.Image:
    mask = Image.new("L", (MASTER_SIZE, MASTER_SIZE), 0)
    draw = ImageDraw.Draw(mask)
    draw.polygon(
        points(
            [
                (5.0, 1.7),
                (27.0, 1.7),
                (30.3, 5.0),
                (30.3, 27.0),
                (27.0, 30.3),
                (5.0, 30.3),
                (1.7, 27.0),
                (1.7, 5.0),
            ]
        ),
        fill=255,
    )
    return mask


def draw_ashen_plate() -> Image.Image:
    """Draw a flat, weathered prompt plate inspired by DS3's restrained UI."""

    image = Image.new("RGBA", (MASTER_SIZE, MASTER_SIZE), (0, 0, 0, 0))
    plate = points([(2.9, 2.5), (29.0, 2.8), (29.3, 29.0), (2.7, 29.3)])

    mask = Image.new("L", image.size, 0)
    ImageDraw.Draw(mask).polygon(plate, fill=232)

    shadow = Image.new("RGBA", image.size, (0, 0, 0, 0))
    shadow_alpha = mask.filter(ImageFilter.GaussianBlur(u(0.65)))
    shadow.putalpha(shadow_alpha.point(lambda a: round(a * 0.54)))
    image.alpha_composite(shadow)

    face = Image.new("RGBA", image.size, (7, 7, 8, 226))
    face.putalpha(mask)
    image.alpha_composite(face)

    # Large, soft patches break up the surface without creating a shiny bevel.
    patina = Image.new("RGBA", image.size, (0, 0, 0, 0))
    patina_draw = ImageDraw.Draw(patina)
    for x0, y0, x1, y1, color in [
        (3.5, 4.0, 20.0, 15.5, (55, 49, 40, 25)),
        (12.0, 13.0, 29.0, 28.0, (40, 43, 46, 30)),
        (1.0, 20.0, 17.0, 32.0, (72, 62, 46, 16)),
        (19.0, 1.5, 31.0, 13.0, (88, 78, 59, 12)),
    ]:
        patina_draw.ellipse((u(x0), u(y0), u(x1), u(y1)), fill=color)
    patina = patina.filter(ImageFilter.GaussianBlur(u(2.4)))
    patina.putalpha(Image.composite(patina.getchannel("A"), Image.new("L", image.size, 0), mask))
    image.alpha_composite(patina)

    draw = ImageDraw.Draw(image)
    closed = plate + [plate[0]]
    # Thin, low-contrast iron frame. It is deliberately flat rather than beveled.
    draw.line(closed, fill=(32, 30, 28, 245), width=u(1.35), joint="curve")
    draw.line(closed, fill=(105, 96, 82, 215), width=u(0.58), joint="curve")
    inner = points([(4.2, 4.0), (27.7, 4.2), (27.9, 27.7), (4.0, 28.0), (4.2, 4.0)])
    draw.line(inner, fill=(54, 51, 47, 150), width=u(0.26), joint="curve")

    # Sparse chips and worn sections keep the border from reading as molded plastic.
    wear = [
        ((7.0, 2.45), (9.3, 2.58), (18, 17, 17, 215)),
        ((21.8, 2.68), (23.0, 2.72), (19, 18, 18, 180)),
        ((29.12, 9.2), (29.18, 11.0), (18, 17, 17, 190)),
        ((29.20, 23.5), (29.23, 25.1), (20, 19, 18, 180)),
        ((18.5, 29.12), (20.5, 29.14), (18, 17, 17, 205)),
        ((2.80, 18.2), (2.78, 20.0), (17, 17, 18, 185)),
    ]
    for start, end, color in wear:
        draw.line(points([start, end]), fill=color, width=u(0.72))

    draw.line(points([(4.8, 3.25), (16.2, 3.38)]), fill=(151, 137, 112, 78), width=u(0.18))
    draw.line(points([(3.45, 5.0), (3.27, 14.8)]), fill=(131, 122, 106, 60), width=u(0.16))
    return image


def draw_worn_translucent_plate() -> Image.Image:
    """Draw the final translucent used-metal plate in either color option."""

    ember = FRAME_STYLE == "worn_ember"
    edge = (151, 84, 43, 220) if ember else (111, 102, 87, 214)
    edge_dark = (70, 42, 27, 224) if ember else (52, 49, 45, 220)
    glint = (201, 132, 72, 112) if ember else (158, 145, 119, 94)
    rust = (139, 66, 31, 54) if ember else (105, 71, 43, 34)

    image = Image.new("RGBA", (MASTER_SIZE, MASTER_SIZE), (0, 0, 0, 0))
    plate = points([(2.9, 2.5), (29.0, 2.8), (29.3, 29.0), (2.7, 29.3)])
    mask = Image.new("L", image.size, 0)
    ImageDraw.Draw(mask).polygon(plate, fill=205)

    shadow = Image.new("RGBA", image.size, (0, 0, 0, 0))
    shadow_alpha = mask.filter(ImageFilter.GaussianBlur(u(0.55)))
    shadow.putalpha(shadow_alpha.point(lambda a: round(a * 0.40)))
    image.alpha_composite(shadow)

    face = Image.new("RGBA", image.size, (7, 7, 8, 0))
    # The maximum face opacity is about 72%; the game menu can remain visible.
    face.putalpha(mask.point(lambda a: round(a * 0.90)))
    image.alpha_composite(face)

    texture = Image.new("RGBA", image.size, (0, 0, 0, 0))
    td = ImageDraw.Draw(texture)
    patches = [
        (3.0, 3.5, 17.5, 14.0, rust),
        (12.0, 9.0, 30.0, 25.0, (76, 65, 50, 25)),
        (1.5, 19.0, 18.0, 31.0, (112, 69, 37, 27) if ember else (78, 65, 49, 23)),
        (19.0, 1.5, 31.0, 13.5, (150, 94, 46, 24) if ember else (120, 103, 75, 19)),
    ]
    for x0, y0, x1, y1, color in patches:
        td.ellipse((u(x0), u(y0), u(x1), u(y1)), fill=color)
    texture = texture.filter(ImageFilter.GaussianBlur(u(2.0)))
    texture.putalpha(Image.composite(texture.getchannel("A"), Image.new("L", image.size, 0), mask))
    image.alpha_composite(texture)

    draw = ImageDraw.Draw(image)
    closed = plate + [plate[0]]
    # Uneven layered edges suggest oxidized sheet metal, not a smooth bevel.
    draw.line(closed, fill=edge_dark, width=u(1.35), joint="curve")
    draw.line(closed, fill=edge, width=u(0.62), joint="curve")
    inner = points([(4.25, 4.05), (27.65, 4.25), (27.85, 27.65), (4.05, 27.95), (4.25, 4.05)])
    draw.line(inner, fill=edge_dark, width=u(0.30), joint="curve")

    draw.line(points([(4.7, 3.20), (11.2, 3.29)]), fill=glint, width=u(0.24))
    draw.line(points([(14.2, 3.36), (20.7, 3.42)]), fill=glint, width=u(0.16))
    draw.line(points([(3.38, 5.0), (3.28, 11.5)]), fill=glint, width=u(0.18))

    # Breaks, dark pits, and orange oxidation appear mainly along exposed edges.
    chips = [
        ((7.0, 2.48), (9.2, 2.58), (17, 16, 16, 205), 0.74),
        ((21.8, 2.69), (23.2, 2.72), (18, 17, 17, 190), 0.68),
        ((29.13, 9.2), (29.18, 11.0), (17, 16, 16, 190), 0.70),
        ((29.20, 23.4), (29.23, 25.0), rust, 0.64),
        ((18.5, 29.13), (20.6, 29.14), (18, 17, 17, 200), 0.72),
        ((2.80, 18.1), (2.78, 20.1), rust, 0.62),
    ]
    for start, end, color, width in chips:
        draw.line(points([start, end]), fill=color, width=u(width))

    pits = [
        (5.7, 5.4, 0.34),
        (25.9, 7.0, 0.28),
        (7.2, 25.3, 0.27),
        (24.0, 23.8, 0.34),
        (17.8, 6.2, 0.22),
    ]
    pit_color = (113, 55, 29, 92) if ember else (92, 67, 43, 72)
    for x, y, radius in pits:
        draw.ellipse((u(x - radius), u(y - radius), u(x + radius), u(y + radius)), fill=pit_color)
    return image


def draw_plate() -> Image.Image:
    if FRAME_STYLE in {"worn_ember", "worn_ashen"}:
        return draw_worn_translucent_plate()
    if FRAME_STYLE == "ashen":
        return draw_ashen_plate()

    image = Image.new("RGBA", (MASTER_SIZE, MASTER_SIZE), (0, 0, 0, 0))
    mask = plate_mask()

    shadow = Image.new("RGBA", image.size, (0, 0, 0, 0))
    shadow_alpha = mask.filter(ImageFilter.GaussianBlur(u(1.15)))
    shadow_alpha = shadow_alpha.transform(
        image.size,
        Image.Transform.AFFINE,
        (1, 0, 0, 0, 1, -u(0.7)),
        resample=Image.Resampling.BICUBIC,
    )
    shadow.putalpha(shadow_alpha.point(lambda a: round(a * 0.72)))
    image.alpha_composite(shadow)

    gradient = Image.new("RGBA", image.size)
    grad_pixels = gradient.load()
    for y in range(MASTER_SIZE):
        t = y / (MASTER_SIZE - 1)
        r = round(30 * (1 - t) + 7 * t)
        g = round(29 * (1 - t) + 7 * t)
        b = round(30 * (1 - t) + 9 * t)
        a = round(244 * (1 - t) + 249 * t)
        for x in range(MASTER_SIZE):
            grad_pixels[x, y] = (r, g, b, a)
    gradient.putalpha(Image.composite(gradient.getchannel("A"), Image.new("L", image.size, 0), mask))
    image.alpha_composite(gradient)

    draw = ImageDraw.Draw(image)
    outer = points(
        [
            (5.0, 1.7),
            (27.0, 1.7),
            (30.3, 5.0),
            (30.3, 27.0),
            (27.0, 30.3),
            (5.0, 30.3),
            (1.7, 27.0),
            (1.7, 5.0),
            (5.0, 1.7),
        ]
    )
    inner = points(
        [
            (5.8, 3.1),
            (26.2, 3.1),
            (28.9, 5.8),
            (28.9, 26.2),
            (26.2, 28.9),
            (5.8, 28.9),
            (3.1, 26.2),
            (3.1, 5.8),
            (5.8, 3.1),
        ]
    )
    if FRAME_STYLE == "subtle":
        # A deliberately quiet graphite frame. Both structural strokes are
        # exactly half the width of the antique-brass version.
        draw.line(outer, fill=(24, 25, 28, 245), width=u(1.2), joint="curve")
        draw.line(outer, fill=(91, 92, 94, 230), width=u(0.575), joint="curve")
        draw.line(inner, fill=(57, 58, 61, 175), width=u(0.30), joint="curve")
        draw.line(
            points([(6.2, 3.55), (25.8, 3.55), (28.35, 6.1)]),
            fill=(127, 126, 122, 90),
            width=u(0.22),
            joint="curve",
        )
        draw.line(
            points([(28.35, 25.9), (25.8, 28.45), (6.2, 28.45)]),
            fill=(22, 23, 26, 155),
            width=u(0.24),
            joint="curve",
        )
    else:
        draw.line(outer, fill=(40, 29, 17, 255), width=u(2.4), joint="curve")
        draw.line(outer, fill=BRASS, width=u(1.15), joint="curve")
        draw.line(inner, fill=(89, 67, 40, 225), width=u(0.55), joint="curve")
        draw.line(
            points([(6.2, 3.55), (25.8, 3.55), (28.35, 6.1)]),
            fill=(224, 184, 109, 150),
            width=u(0.42),
            joint="curve",
        )
        draw.line(
            points([(28.35, 25.9), (25.8, 28.45), (6.2, 28.45)]),
            fill=(42, 31, 19, 210),
            width=u(0.48),
            joint="curve",
        )
    return image


def composite_symbol(base: Image.Image, symbol: Image.Image) -> Image.Image:
    shadow = Image.new("RGBA", base.size, (0, 0, 0, 0))
    blur = 0.24 if FRAME_STYLE == "ashen" else 0.45
    alpha = symbol.getchannel("A").filter(ImageFilter.GaussianBlur(u(blur)))
    shadow_offset_x = 0.32 if FRAME_STYLE == "ashen" else 0.55
    shadow_offset_y = 0.42 if FRAME_STYLE == "ashen" else 0.75
    alpha = alpha.transform(
        base.size,
        Image.Transform.AFFINE,
        (1, 0, -u(shadow_offset_x), 0, 1, -u(shadow_offset_y)),
        resample=Image.Resampling.BICUBIC,
    )
    shadow_strength = 0.62 if FRAME_STYLE == "ashen" else 0.9
    shadow.putalpha(alpha.point(lambda a: round(a * shadow_strength)))
    base.alpha_composite(shadow)
    base.alpha_composite(symbol)
    return base


def letter_symbol(value: str) -> Image.Image:
    symbol = Image.new("RGBA", (MASTER_SIZE, MASTER_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(symbol)
    draw.text(
        (u(16), u(15.8)),
        value,
        font=font(FONT_BOLD, 15.4),
        fill=PARCHMENT,
        stroke_width=u(0.18),
        stroke_fill=(255, 250, 232, 230),
        anchor="mm",
    )
    return symbol


def esc_symbol() -> Image.Image:
    symbol = Image.new("RGBA", (MASTER_SIZE, MASTER_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(symbol)
    draw.text(
        (u(16), u(16.2)),
        "ESC",
        font=font(FONT_BOLD, 7.15),
        fill=PARCHMENT,
        stroke_width=u(0.12),
        stroke_fill=(255, 250, 232, 230),
        anchor="mm",
    )
    return symbol


def arrow_symbol(direction: str) -> Image.Image:
    symbol = Image.new("RGBA", (MASTER_SIZE, MASTER_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(symbol)
    shape = points(
        [
            (8.3, 13.6),
            (16.2, 13.6),
            (16.2, 9.4),
            (23.8, 16.0),
            (16.2, 22.6),
            (16.2, 18.4),
            (8.3, 18.4),
        ]
    )
    draw.polygon(shape, fill=PARCHMENT)
    draw.line(shape + [shape[0]], fill=(255, 250, 230, 230), width=u(0.35), joint="curve")
    angles = {"right": 0, "down": -90, "left": 180, "up": 90}
    return symbol.rotate(angles[direction], resample=Image.Resampling.BICUBIC, center=(u(16), u(16)))


def space_symbol() -> Image.Image:
    symbol = Image.new("RGBA", (MASTER_SIZE, MASTER_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(symbol)
    path = points([(8.0, 14.1), (8.0, 19.5), (24.0, 19.5), (24.0, 14.1)])
    draw.line(path, fill=PARCHMENT, width=u(1.75), joint="curve")
    draw.line(
        points([(10.2, 21.8), (21.8, 21.8)]),
        fill=(126, 94, 53, 190),
        width=u(0.55),
    )
    return symbol


def draw_mouse(draw: ImageDraw.ImageDraw, *, left: float, top: float, width: float, height: float, button: str) -> None:
    x0, y0, x1, y1 = u(left), u(top), u(left + width), u(top + height)
    radius = u(width * 0.45)
    split_y = u(top + height * 0.36)
    center_x = u(left + width / 2)

    draw.rounded_rectangle(
        (x0, y0, x1, y1),
        radius=radius,
        fill=(17, 17, 20, 235),
        outline=PARCHMENT,
        width=u(1.05),
    )
    if button in {"left", "right"}:
        bx0 = x0 + u(1.25) if button == "left" else center_x
        bx1 = center_x if button == "left" else x1 - u(1.25)
        if FRAME_STYLE == "framed":
            button_fill = (185, 139, 70, 245)
        elif FRAME_STYLE == "subtle":
            button_fill = (148, 147, 141, 245)
        elif FRAME_STYLE == "ashen":
            button_fill = (137, 123, 96, 238)
        elif FRAME_STYLE == "worn_ember":
            button_fill = (158, 88, 48, 225)
        elif FRAME_STYLE == "worn_ashen":
            button_fill = (132, 120, 98, 220)
        else:
            button_fill = PARCHMENT
        draw.rectangle((bx0, y0 + u(1.25), bx1, split_y), fill=button_fill)
    draw.line((x0 + u(0.9), split_y, x1 - u(0.9), split_y), fill=PARCHMENT_DIM, width=u(0.72))
    draw.line((center_x, y0 + u(0.75), center_x, split_y), fill=PARCHMENT_DIM, width=u(0.65))
    draw.rounded_rectangle(
        (center_x - u(0.62), y0 + u(3.4), center_x + u(0.62), y0 + u(6.3)),
        radius=u(0.55),
        fill=PARCHMENT,
    )
    draw.rounded_rectangle(
        (x0, y0, x1, y1),
        radius=radius,
        outline=(252, 245, 220, 255),
        width=u(0.72),
    )


def mouse_symbol(button: str) -> Image.Image:
    symbol = Image.new("RGBA", (MASTER_SIZE, MASTER_SIZE), (0, 0, 0, 0))
    draw_mouse(ImageDraw.Draw(symbol), left=10.6, top=6.7, width=10.8, height=18.7, button=button)
    return symbol


def shift_mouse_symbol(button: str) -> Image.Image:
    symbol = Image.new("RGBA", (MASTER_SIZE, MASTER_SIZE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(symbol)
    shift = points(
        [
            (4.8, 15.2),
            (9.5, 10.2),
            (14.2, 15.2),
            (11.8, 15.2),
            (11.8, 21.7),
            (7.2, 21.7),
            (7.2, 15.2),
        ]
    )
    draw.polygon(shift, fill=PARCHMENT)
    draw.line(shift + [shift[0]], fill=(255, 250, 232, 230), width=u(0.3), joint="curve")
    draw_mouse(draw, left=17.0, top=9.0, width=8.5, height=14.8, button=button)
    if FRAME_STYLE == "framed":
        plus_color = BRASS_LIGHT
    elif FRAME_STYLE == "subtle":
        plus_color = (188, 186, 177, 255)
    elif FRAME_STYLE == "ashen":
        plus_color = (184, 174, 151, 245)
    elif FRAME_STYLE == "worn_ember":
        plus_color = (192, 124, 73, 235)
    elif FRAME_STYLE == "worn_ashen":
        plus_color = (174, 161, 137, 230)
    else:
        plus_color = PARCHMENT
    draw.line(points([(14.25, 15.8), (15.75, 15.8)]), fill=plus_color, width=u(0.55))
    draw.line(points([(15.0, 15.05), (15.0, 16.55)]), fill=plus_color, width=u(0.55))
    return symbol


def render(symbol_factory: Callable[[], Image.Image]) -> Image.Image:
    if FRAME_STYLE == "frameless":
        return composite_symbol(
            Image.new("RGBA", (MASTER_SIZE, MASTER_SIZE), (0, 0, 0, 0)),
            symbol_factory(),
        )
    rendered = composite_symbol(draw_plate(), symbol_factory())
    if FRAME_STYLE in {"worn_ember", "worn_ashen"}:
        # Reduce the whole finished prompt slightly; glyphs remain more opaque
        # than the face because their source alpha begins at 255.
        rendered.putalpha(rendered.getchannel("A").point(lambda a: round(a * 0.88)))
    return rendered


TARGETS: dict[str, tuple[str, Callable[[], Image.Image]]] = {
    "KG_OK": ("E — Confirm", lambda: letter_symbol("E")),
    "KG_Cancel": ("Q — Cancel", lambda: letter_symbol("Q")),
    "KG_R_L": ("R — Use item", lambda: letter_symbol("R")),
    "KG_R_U": ("F — Two-hand", lambda: letter_symbol("F")),
    "KG_R3": ("C — Lock on", lambda: letter_symbol("C")),
    "KG_TP_L": ("G — Gestures", lambda: letter_symbol("G")),
    "KG_Start": ("Esc — Menu", esc_symbol),
    "KG_TP_R": ("Esc — Menu (alternate)", esc_symbol),
    "KG_L_U": ("Up arrow", lambda: arrow_symbol("up")),
    "KG_L_D": ("Down arrow", lambda: arrow_symbol("down")),
    "KG_L_L": ("Left arrow", lambda: arrow_symbol("left")),
    "KG_L_R": ("Right arrow", lambda: arrow_symbol("right")),
    "KG_L3": ("Space", space_symbol),
    "KG_R1": ("LMB — Attack", lambda: mouse_symbol("left")),
    "KG_L1": ("RMB — Block", lambda: mouse_symbol("right")),
    "KG_R2": ("Shift + LMB", lambda: shift_mouse_symbol("left")),
    "KG_L2": ("Shift + RMB", lambda: shift_mouse_symbol("right")),
}


def build_preview(rendered: dict[str, Image.Image]) -> None:
    ordered = [
        "KG_OK",
        "KG_Cancel",
        "KG_R_L",
        "KG_R_U",
        "KG_R3",
        "KG_TP_L",
        "KG_Start",
        "KG_L3",
        "KG_L_U",
        "KG_L_D",
        "KG_L_L",
        "KG_L_R",
        "KG_R1",
        "KG_L1",
        "KG_R2",
        "KG_L2",
    ]
    canvas = Image.new("RGB", (1280, 960), (13, 13, 16))
    draw = ImageDraw.Draw(canvas)
    title_font = ImageFont.truetype(str(FONT_BOLD), 40)
    subtitle_font = ImageFont.truetype(str(FONT_REGULAR), 20)
    label_font = ImageFont.truetype(str(FONT_BOLD), 18)
    small_font = ImageFont.truetype(str(FONT_REGULAR), 14)
    if FRAME_STYLE == "frameless":
        title_text = "ORIGINAL FRAMELESS KEYBOARD & MOUSE PROMPTS"
        subtitle_text = "Transparent icon-only edition • no keycap plate and no yellow outline"
    elif FRAME_STYLE == "subtle":
        title_text = "ORIGINAL THIN-FRAME KEYBOARD & MOUSE PROMPTS"
        subtitle_text = "Dark graphite frame • at least 2× thinner • subdued for the DS3 interface"
    elif FRAME_STYLE == "ashen":
        title_text = "ORIGINAL ASHEN-IRON KEYBOARD & MOUSE PROMPTS"
        subtitle_text = "Flat weathered iron • worn taupe edging • aged ivory • designed for DS3's UI mood"
    elif FRAME_STYLE == "worn_ember":
        title_text = "WORN METAL KEYBOARD & MOUSE PROMPTS — RUSTED EMBER"
        subtitle_text = "Semi-transparent iron • burnt-orange oxidation • worn edges • aged ivory"
    elif FRAME_STYLE == "worn_ashen":
        title_text = "WORN METAL KEYBOARD & MOUSE PROMPTS — ASHEN IRON"
        subtitle_text = "Semi-transparent iron • subdued taupe-gray metal • worn edges • aged ivory"
    else:
        title_text = "ORIGINAL KEYBOARD & MOUSE PROMPTS"
        subtitle_text = "Clean-room artwork • antique iron, aged brass, and consistent typography"
    title_size = 40
    title_font = ImageFont.truetype(str(FONT_BOLD), title_size)
    while draw.textbbox((0, 0), title_text, font=title_font)[2] > 1180 and title_size > 26:
        title_size -= 1
        title_font = ImageFont.truetype(str(FONT_BOLD), title_size)
    draw.text((640, 42), title_text, font=title_font, fill=(235, 224, 198), anchor="mm")
    draw.text(
        (640, 82),
        subtitle_text,
        font=subtitle_font,
        fill=(164, 150, 122),
        anchor="mm",
    )
    for index, name in enumerate(ordered):
        row, col = divmod(index, 4)
        x = 52 + col * 304
        y = 122 + row * 194
        cell = (x, y, x + 264, y + 158)
        cell_outline = (43, 43, 49) if FRAME_STYLE != "framed" else (65, 53, 36)
        draw.rounded_rectangle(cell, radius=14, fill=(21, 21, 25), outline=cell_outline, width=2)
        icon = rendered[name].resize((112, 112), Image.Resampling.LANCZOS)
        canvas.paste(icon, (x + 18, y + 21), icon)
        label = TARGETS[name][0]
        main = label.split(" — ", 1)[0]
        detail = label.split(" — ", 1)[1] if " — " in label else ""
        draw.text((x + 145, y + 53), main, font=label_font, fill=(238, 228, 202), anchor="lm")
        draw.text((x + 145, y + 82), detail, font=small_font, fill=(156, 144, 119), anchor="lm")
        draw.text((x + 145, y + 111), name, font=small_font, fill=(103, 96, 84), anchor="lm")
    draw.text(
        (640, 924),
        "Rendered from original vector-like geometry; final in-game textures are 32 × 32 BC7.",
        font=small_font,
        fill=(117, 108, 92),
        anchor="mm",
    )
    if FRAME_STYLE == "frameless":
        preview_name = "DS3_Original_Icons_Frameless_Preview.png"
    elif FRAME_STYLE == "subtle":
        preview_name = "DS3_Original_Icons_ThinFrame_Preview.png"
    elif FRAME_STYLE == "ashen":
        preview_name = "DS3_Original_Icons_AshenIron_Nexus_Preview.png"
    elif FRAME_STYLE == "worn_ember":
        preview_name = "DS3_Original_Icons_RustedEmber_Nexus_Preview.png"
    elif FRAME_STYLE == "worn_ashen":
        preview_name = "DS3_Original_Icons_WornAshen_Nexus_Preview.png"
    else:
        preview_name = "DS3_Original_Keycaps_Preview.png"
    canvas.save(PREVIEW_DIR / preview_name, optimize=True)

    qa = Image.new("RGB", (1024, 600), (32, 32, 36))
    qa_draw = ImageDraw.Draw(qa)
    qa_title = ImageFont.truetype(str(FONT_BOLD), 26)
    qa_label = ImageFont.truetype(str(FONT_REGULAR), 14)
    qa_draw.text((512, 30), "32 PX PIXEL QA — NEAREST-NEIGHBOR ENLARGEMENT", font=qa_title, fill=(230, 220, 196), anchor="mm")
    for index, name in enumerate(ordered):
        row, col = divmod(index, 8)
        x = 24 + col * 124
        y = 72 + row * 252
        final_icon = rendered[name].resize((FINAL_SIZE, FINAL_SIZE), Image.Resampling.LANCZOS)
        enlarged = final_icon.resize((160, 160), Image.Resampling.NEAREST)
        enlarged.thumbnail((112, 112), Image.Resampling.NEAREST)
        qa.paste(enlarged, (x + 6, y), enlarged)
        qa_draw.text((x + 62, y + 130), TARGETS[name][0].split(" — ", 1)[0], font=qa_label, fill=(205, 194, 170), anchor="mm")
        qa_draw.text((x + 62, y + 151), name, font=qa_label, fill=(126, 117, 101), anchor="mm")
    if FRAME_STYLE == "frameless":
        qa_name = "DS3_Original_Icons_Frameless_32px_QA.png"
    elif FRAME_STYLE == "subtle":
        qa_name = "DS3_Original_Icons_ThinFrame_32px_QA.png"
    elif FRAME_STYLE == "ashen":
        qa_name = "DS3_Original_Icons_AshenIron_32px_QA.png"
    elif FRAME_STYLE == "worn_ember":
        qa_name = "DS3_Original_Icons_RustedEmber_32px_QA.png"
    elif FRAME_STYLE == "worn_ashen":
        qa_name = "DS3_Original_Icons_WornAshen_32px_QA.png"
    else:
        qa_name = "DS3_Original_Keycaps_32px_QA.png"
    qa.save(PREVIEW_DIR / qa_name, optimize=True)


def main() -> None:
    parser = argparse.ArgumentParser()
    style_group = parser.add_mutually_exclusive_group()
    style_group.add_argument("--frameless", action="store_true", help="render icons without the keycap plate")
    style_group.add_argument("--subtle-frame", action="store_true", help="render a thin graphite keycap frame")
    style_group.add_argument("--ashen", action="store_true", help="render a flat weathered iron DS3-style frame")
    style_group.add_argument("--worn-ember", action="store_true", help="render translucent rusted-orange metal")
    style_group.add_argument("--worn-ashen", action="store_true", help="render translucent taupe-gray metal")
    parser.add_argument("--output-root", type=Path, help="directory for masters, PNGs, and previews")
    args = parser.parse_args()

    global ROOT, MASTER_DIR, PNG_DIR, PREVIEW_DIR, FRAME_STYLE
    if args.output_root:
        ROOT = args.output_root.resolve()
        MASTER_DIR = ROOT / "masters"
        PNG_DIR = ROOT / "png"
        PREVIEW_DIR = ROOT / "preview"
    if args.frameless:
        FRAME_STYLE = "frameless"
    elif args.subtle_frame:
        FRAME_STYLE = "subtle"
    elif args.ashen:
        FRAME_STYLE = "ashen"
    elif args.worn_ember:
        FRAME_STYLE = "worn_ember"
    elif args.worn_ashen:
        FRAME_STYLE = "worn_ashen"
    else:
        FRAME_STYLE = "framed"

    for directory in (MASTER_DIR, PNG_DIR, PREVIEW_DIR):
        directory.mkdir(parents=True, exist_ok=True)

    rendered: dict[str, Image.Image] = {}
    for target, (_, factory) in TARGETS.items():
        master = render(factory)
        final = master.resize((FINAL_SIZE, FINAL_SIZE), Image.Resampling.LANCZOS)
        master.save(MASTER_DIR / f"{target}.png", optimize=True)
        final.save(PNG_DIR / f"{target}.png", optimize=True)
        rendered[target] = master

    build_preview(rendered)
    print(f"Rendered {len(rendered)} {FRAME_STYLE} target textures.")
    if FRAME_STYLE == "frameless":
        preview_name = "DS3_Original_Icons_Frameless_Preview.png"
    elif FRAME_STYLE == "subtle":
        preview_name = "DS3_Original_Icons_ThinFrame_Preview.png"
    elif FRAME_STYLE == "ashen":
        preview_name = "DS3_Original_Icons_AshenIron_Nexus_Preview.png"
    elif FRAME_STYLE == "worn_ember":
        preview_name = "DS3_Original_Icons_RustedEmber_Nexus_Preview.png"
    elif FRAME_STYLE == "worn_ashen":
        preview_name = "DS3_Original_Icons_WornAshen_Nexus_Preview.png"
    else:
        preview_name = "DS3_Original_Keycaps_Preview.png"
    print(PREVIEW_DIR / preview_name)


if __name__ == "__main__":
    main()
