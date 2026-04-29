import pkgutil
import importlib

__all__ = []

for m in pkgutil.iter_modules(__path__):
    module = importlib.import_module(f'{__name__}.{m.name}')
    __all__.append(m.name)
