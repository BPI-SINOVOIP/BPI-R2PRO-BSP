import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { SnapShotComponent } from './snap-shot.component';

describe('SnapShotComponent', () => {
  let component: SnapShotComponent;
  let fixture: ComponentFixture<SnapShotComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ SnapShotComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(SnapShotComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
