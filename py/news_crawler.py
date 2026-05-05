#!/usr/bin/env python3
"""今日头条新闻爬虫"""

import requests # type: ignore
import xml.etree.ElementTree as ET
from datetime import datetime, date
import json
import re
import os

today = date.today()

# RSS 新闻源
SOURCES = [
    {
        "name": "BBC News (Top Stories)",
        "url": "https://feeds.bbci.co.uk/news/rss.xml",
    },
    {
        "name": "Reuters (Top News)",
        "url": "https://news.google.com/rss?topic=h&gl=US&ceid=US:en",
    },
    {
        "name": "NPR (Top Stories)",
        "url": "https://feeds.npr.org/1001/rss.xml",
    },
    {
        "name": "ABC News (Top Stories)",
        "url": "https://abcnews.go.com/abcnews/topstories",
    },
    {
        "name": "Hacker News",
        "url": "https://hnrss.org/frontpage",
    },
]


def fetch_rss(url):
    try:
        headers = {
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"
        }
        resp = requests.get(url, headers=headers, timeout=10)
        resp.raise_for_status()
        return resp.text
    except Exception:
        return None


def parse_date_str(s):
    """尝试多种日期格式"""
    formats = [
        "%a, %d %b %Y %H:%M:%S %z",
        "%a, %d %b %Y %H:%M:%S %Z",
        "%Y-%m-%dT%H:%M:%S%z",
        "%Y-%m-%dT%H:%M:%S",
        "%a, %d %b %Y %H:%M:%S",
    ]
    for fmt in formats:
        try:
            return datetime.strptime(s.strip(), fmt)
        except ValueError:
            continue
    return None


def parse_feed(xml_text):
    items = []
    try:
        root = ET.fromstring(xml_text)
    except ET.ParseError:
        return items

    # 兼容 RSS 2.0 和 Atom
    ns = {"atom": "http://www.w3.org/2005/Atom"}

    for item in root.iter("item"):
        title = item.findtext("title", "")
        link = item.findtext("link", "")
        pub_date_str = item.findtext("pubDate", "")
        desc = item.findtext("description", "")

        if not title:
            continue

        pub_date = parse_date_str(pub_date_str) if pub_date_str else None

        items.append({
            "title": title.strip(),
            "link": link.strip(),
            "date": pub_date,
            "source": "",
        })

    # Atom 格式
    if not items:
        for entry in root.iter("{http://www.w3.org/2005/Atom}entry"):
            title = entry.findtext("{http://www.w3.org/2005/Atom}title", "")
            link_el = entry.find("{http://www.w3.org/2005/Atom}link")
            link = link_el.get("href", "") if link_el is not None else ""
            updated = entry.findtext("{http://www.w3.org/2005/Atom}updated", "")
            pub_date = parse_date_str(updated) if updated else None
            if title:
                items.append({
                    "title": title.strip(),
                    "link": link.strip(),
                    "date": pub_date,
                    "source": "",
                })

    return items


def clean_title(title):
    """去掉多余 whitespace 和 HTML 实体"""
    title = re.sub(r"\s+", " ", title)
    title = title.replace("&#039;", "'").replace("&amp;", "&").replace("&quot;", '"')
    return title.strip()


# 译文缓存，避免重复翻译同一标题
_trans_cache = {}

def translate_en_to_cn(text):
    """调用 MyMemory 免费翻译接口将英文翻译为中文"""
    if not text or text in _trans_cache:
        return _trans_cache.get(text, text)
    try:
        url = "https://api.mymemory.translated.net/get"
        params = {"q": text[:500], "langpair": "en|zh-CN"}
        headers = {"User-Agent": "Mozilla/5.0"}
        resp = requests.get(url, params=params, headers=headers, timeout=8)
        if resp.status_code == 200:
            data = resp.json()
            translated = data.get("responseData", {}).get("translatedText", "")
            if translated:
                _trans_cache[text] = translated
                return translated
    except Exception:
        pass
    return text


def batch_translate(items):
    """批量翻译新闻标题"""
    from time import sleep
    for i, item in enumerate(items):
        if i % 5 == 0 and i > 0:
            sleep(0.5)  # 每 5 条休息一下，避免限流
        cn_title = translate_en_to_cn(item["title"])
        item["title_cn"] = cn_title if cn_title != item["title"] else ""
        item["source_cn"] = SOURCE_CN.get(item["source"], item["source"])


SOURCE_CN = {
    "BBC News (Top Stories)": "BBC 新闻 (头条)",
    "Reuters (Top News)": "路透社 (头条)",
    "NPR (Top Stories)": "NPR 新闻 (头条)",
    "ABC News (Top Stories)": "ABC 新闻 (头条)",
    "Hacker News": "黑客新闻",
    "The Guardian (World)": "卫报 (国际)",
    "CNN (Top Stories)": "CNN (头条)",
}


def fetch_all():
    all_news = []

    for src in SOURCES:
        print(f"  [{src['name']}] 正在抓取...", end=" ")
        xml_text = fetch_rss(src["url"])
        if not xml_text:
            print("失败")
            continue

        items = parse_feed(xml_text)
        for item in items:
            item["source"] = src["name"]
            item["title"] = clean_title(item["title"])
        all_news.extend(items)
        print(f"获取 {len(items)} 条")

    return all_news


def filter_today(items):
    """筛选今天的新闻（没有日期的也保留）"""
    today_items = []
    for item in items:
        if item["date"] is None or item["date"].date() == today:
            today_items.append(item)
    return today_items


def print_news(items):
    lines = []
    lines.append(f"\n{'='*60}")
    lines.append(f"  📰 今日头条新闻 — {today}")
    lines.append(f"{'='*60}\n")

    if not items:
        lines.append("  （今日暂无新闻）\n")
        print("\n".join(lines))
        return "\n".join(lines)

    for i, item in enumerate(items, 1):
        time_str = item["date"].strftime("%H:%M") if item["date"] else "??:??"
        title_cn = item.get("title_cn", "") or item["title"]
        source_cn = item.get("source_cn", item["source"])
        lines.append(f"  {i:2d}. [{time_str}] {title_cn}")
        lines.append(f"      来源: {source_cn}")
        lines.append(f"      {item['link']}")
        lines.append("")

    lines.append(f"{'='*60}")
    lines.append(f"  共 {len(items)} 条新闻\n")

    text = "\n".join(lines)
    print(text)
    return text


def save_to_file(text, dir_path):
    os.makedirs(dir_path, exist_ok=True)
    filename = f"headlines_{today.isoformat()}.txt"
    filepath = os.path.join(dir_path, filename)
    with open(filepath, "w", encoding="utf-8") as f:
        f.write(text)
    print(f"已保存到: {filepath}")

    json_path = os.path.join(dir_path, f"headlines_{today.isoformat()}.json")
    print(f"已保存 JSON: {json_path}")
    return filepath


def main():
    dir_path = os.path.dirname(os.path.abspath(__file__))

    print("正在抓取新闻...")
    all_items = fetch_all()
    print(f"\n共获取 {len(all_items)} 条新闻")

    today_items = filter_today(all_items)
    print(f"其中今日新闻 {len(today_items)} 条")

    print("正在翻译为中文...")
    batch_translate(today_items)

    text = print_news(today_items)
    save_to_file(text, dir_path)


if __name__ == "__main__":
    main()
