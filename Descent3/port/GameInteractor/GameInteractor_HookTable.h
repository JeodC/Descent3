DEFINE_HOOK(OnDatabaseCreated, ())
DEFINE_HOOK(OnMovieEnd, ())
DEFINE_HOOK(OnMovieFrame, (int x, int y, int width, int height))
DEFINE_HOOK(ShouldVanillaBehavior, (GIVanillaBehavior flag, bool *should, va_list originalArgs))
