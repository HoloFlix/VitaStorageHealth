from PIL import Image, ImageDraw

# 1. Create Icon (icon0.png - 128x128 8-bit indexed PNG)
icon = Image.new("RGB", (128, 128), color=(20, 24, 33))
d_icon = ImageDraw.Draw(icon)
d_icon.rectangle([34, 24, 94, 104], fill=(41, 128, 185), outline=(52, 152, 219), width=3)
d_icon.rectangle([46, 24, 82, 40], fill=(20, 24, 33))  # SD notch
d_icon.text((45, 60), "SH", fill=(255, 255, 255))
icon_p = icon.convert("P", palette=Image.Palette.ADAPTIVE, colors=256)
icon_p.save("icon0.png")

# 2. Create LiveArea Background (bg0.png - 840x500 8-bit indexed PNG)
bg = Image.new("RGB", (840, 500), color=(15, 17, 23))
d_bg = ImageDraw.Draw(bg)
for x in range(0, 840, 40):
    d_bg.line([(x, 0), (x, 500)], fill=(25, 30, 40), width=1)
d_bg.text((60, 60), "STORAGE HEALTH", fill=(52, 152, 219))
d_bg.text((60, 90), "System Memory Diagnostics & Repair Utility", fill=(149, 165, 166))
bg_p = bg.convert("P", palette=Image.Palette.ADAPTIVE, colors=256)
bg_p.save("bg0.png")

# 3. Create Startup Graphic (startup.png - 280x158 8-bit indexed PNG)
startup = Image.new("RGB", (280, 158), color=(30, 39, 46))
d_start = ImageDraw.Draw(startup)
d_start.text((20, 60), "CHECK INTEGRITY", fill=(46, 204, 113))
startup_p = startup.convert("P", palette=Image.Palette.ADAPTIVE, colors=256)
startup_p.save("startup.png")

# 4. Create Loading Screen (pic0.png - 960x544 8-bit indexed PNG)
pic = Image.new("RGB", (960, 544), color=(10, 12, 16))
d_pic = ImageDraw.Draw(pic)
d_pic.text((80, 450), "Loading Storage Health...", fill=(127, 140, 141))
pic_p = pic.convert("P", palette=Image.Palette.ADAPTIVE, colors=256)
pic_p.save("pic0.png")

print("LiveArea assets converted to 8-bit indexed PNGs successfully!")