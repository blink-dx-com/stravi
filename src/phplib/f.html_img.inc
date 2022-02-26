<?php
// HTML > IMG helper
// @module: f.html_img.inc
class f_html_img {
    static function svg_file_load($filename) {
        $text = file_get_contents($filename);
        return $text;
    }
}