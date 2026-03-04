source "https://rubygems.org"

# 使用最新版 Jekyll（不再是 github-pages gem）
gem "jekyll", "~> 4.3.2"

# 你配置中使用的插件
group :jekyll_plugins do
  gem "jekyll-feed"
  gem "jekyll-sitemap"
  gem "jemoji"
  gem "jekyll-paginate"
  gem "jekyll-seo-tag"  # 对应你的 seo: true 配置
end

# Windows 和 JRuby 不支持时区信息，需要额外添加
platforms :mingw, :x64_mingw, :mswin, :jruby do
  gem "tzinfo", ">= 1", "< 3"
  gem "tzinfo-data"
end

# 文件系统监控
gem "wdm", "~> 0.1.1", :platforms => [:mingw, :x64_mingw, :mswin]

# HTTP 服务（用于本地预览）
gem "webrick", "~> 1.8"
