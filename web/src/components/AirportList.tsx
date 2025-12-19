import { useMemo } from 'react';
import {
  DndContext,
  closestCenter,
  KeyboardSensor,
  PointerSensor,
  useSensor,
  useSensors,
  type DragEndEvent,
} from '@dnd-kit/core';
import {
  arrayMove,
  SortableContext,
  sortableKeyboardCoordinates,
  useSortable,
  verticalListSortingStrategy,
} from '@dnd-kit/sortable';
import { CSS } from '@dnd-kit/utilities';
import { getCategoryColorClass, isSpecialEntry } from '../types/config';

interface AirportListProps {
  airports: string[];
  onChange: (airports: string[]) => void;
}

interface SortableAirportProps {
  id: string;
  index: number;
  code: string;
  onRemove: () => void;
  onEdit: (newCode: string) => void;
}

function SortableAirport({ id, index, code, onRemove, onEdit }: SortableAirportProps) {
  const {
    attributes,
    listeners,
    setNodeRef,
    transform,
    transition,
    isDragging,
  } = useSortable({ id });

  const style = {
    transform: CSS.Transform.toString(transform),
    transition,
  };

  const isSpecial = isSpecialEntry(code);
  const colorClass = isSpecial ? getCategoryColorClass(code) : 'bg-gray-500';

  return (
    <div
      ref={setNodeRef}
      style={style}
      className={`flex items-center gap-2 p-2 rounded-lg bg-gray-800 ${
        isDragging ? 'opacity-50 shadow-lg' : ''
      }`}
    >
      {/* Drag handle */}
      <button
        {...attributes}
        {...listeners}
        className="drag-handle p-1 text-gray-500 hover:text-gray-300"
      >
        <svg className="w-4 h-4" fill="currentColor" viewBox="0 0 20 20">
          <path d="M7 2a2 2 0 1 0 .001 4.001A2 2 0 0 0 7 2zm0 6a2 2 0 1 0 .001 4.001A2 2 0 0 0 7 8zm0 6a2 2 0 1 0 .001 4.001A2 2 0 0 0 7 14zm6-8a2 2 0 1 0-.001-4.001A2 2 0 0 0 13 6zm0 2a2 2 0 1 0 .001 4.001A2 2 0 0 0 13 8zm0 6a2 2 0 1 0 .001 4.001A2 2 0 0 0 13 14z" />
        </svg>
      </button>

      {/* LED Index */}
      <span className="text-gray-500 text-sm w-8 text-right">{index + 1}</span>

      {/* LED Color indicator */}
      <div className={`led-indicator ${colorClass}`} />

      {/* Airport code */}
      <input
        type="text"
        value={code}
        onChange={(e) => onEdit(e.target.value.toUpperCase())}
        className="flex-1 bg-gray-700 rounded px-2 py-1 font-mono text-sm uppercase"
        maxLength={10}
      />

      {/* Remove button */}
      <button
        onClick={onRemove}
        className="p-1 text-gray-500 hover:text-red-400"
        title="Remove"
      >
        <svg className="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M6 18L18 6M6 6l12 12" />
        </svg>
      </button>
    </div>
  );
}

export function AirportList({ airports, onChange }: AirportListProps) {
  const sensors = useSensors(
    useSensor(PointerSensor),
    useSensor(KeyboardSensor, {
      coordinateGetter: sortableKeyboardCoordinates,
    })
  );

  // Create stable IDs for each airport position
  const items = useMemo(() =>
    airports.map((code, index) => ({
      id: `airport-${index}`,
      code,
      index,
    })),
    [airports]
  );

  const handleDragEnd = (event: DragEndEvent) => {
    const { active, over } = event;

    if (over && active.id !== over.id) {
      const oldIndex = items.findIndex((item) => item.id === active.id);
      const newIndex = items.findIndex((item) => item.id === over.id);
      onChange(arrayMove(airports, oldIndex, newIndex));
    }
  };

  const handleRemove = (index: number) => {
    const newAirports = [...airports];
    newAirports.splice(index, 1);
    onChange(newAirports);
  };

  const handleEdit = (index: number, newCode: string) => {
    const newAirports = [...airports];
    newAirports[index] = newCode;
    onChange(newAirports);
  };

  const handleAdd = () => {
    onChange([...airports, 'NULL']);
  };

  return (
    <div className="space-y-2">
      <div className="flex items-center justify-between mb-4">
        <h3 className="text-lg font-semibold">Airport LEDs ({airports.length})</h3>
        <button
          onClick={handleAdd}
          className="px-3 py-1 bg-green-600 hover:bg-green-700 rounded-lg text-sm font-medium"
        >
          + Add LED
        </button>
      </div>

      <div className="text-xs text-gray-500 mb-2">
        Drag to reorder. Special entries: VFR, MVFR, IFR, LIFR, WVFR (legend colors), NULL (disabled)
      </div>

      <DndContext
        sensors={sensors}
        collisionDetection={closestCenter}
        onDragEnd={handleDragEnd}
      >
        <SortableContext items={items.map(i => i.id)} strategy={verticalListSortingStrategy}>
          <div className="space-y-1 max-h-96 overflow-y-auto pr-2">
            {items.map((item) => (
              <SortableAirport
                key={item.id}
                id={item.id}
                index={item.index}
                code={item.code}
                onRemove={() => handleRemove(item.index)}
                onEdit={(newCode) => handleEdit(item.index, newCode)}
              />
            ))}
          </div>
        </SortableContext>
      </DndContext>

      {airports.length === 0 && (
        <div className="text-center py-8 text-gray-500">
          No airports configured. Click "Add LED" or import from legacy config.
        </div>
      )}
    </div>
  );
}
